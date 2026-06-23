#pragma once

#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include <CloverNT/API/Expected.hpp>
#include <CloverNT/API/Macros.hpp>
#include <CloverNT/API/Plugin/Plugin.hpp>

namespace CloverNT::Memory {

using Address    = std::uintptr_t;
using ResourceId = std::uint64_t;

enum class ErrorCode : std::uint8_t {
    Unknown = 0,
    RuntimeInactive,
    PluginUnavailable,
    InvalidArgument,
    InvalidHandle,
    NoFreeSlots,
    ModuleNotFound,
    ImportNotFound,
    MemoryNotReadable,
    MemoryNotWritable,
    ProtectionFailed,
    HookInstallFailed,
    HookRemoveFailed,
};

[[nodiscard]] inline auto makeMemoryError(ErrorCode code, std::string message) -> Error {
    return makeError(ErrorCategory::Memory, std::move(message), static_cast<std::int32_t>(code));
}

enum class MemoryProtection : std::uint32_t {
    None          = 0,
    Read          = 1 << 0,
    Write         = 1 << 1,
    Execute       = 1 << 2,
    ReadWrite     = Read | Write,
    ReadExec      = Read | Execute,
    ReadWriteExec = Read | Write | Execute,
};

constexpr auto operator|(const MemoryProtection lhs, const MemoryProtection rhs) noexcept -> MemoryProtection {
    return static_cast<MemoryProtection>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

constexpr auto operator&(const MemoryProtection lhs, const MemoryProtection rhs) noexcept -> MemoryProtection {
    return static_cast<MemoryProtection>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

constexpr auto hasFlag(const MemoryProtection value, const MemoryProtection flag) noexcept -> bool {
    return (value & flag) == flag;
}

class CloverNT_API Module {
public:
    Module() = default;
    Module(std::string name, Address base, std::size_t size);

    [[nodiscard]] auto name() const noexcept -> std::string const&;
    [[nodiscard]] auto base() const noexcept -> Address;
    [[nodiscard]] auto size() const noexcept -> std::size_t;
    [[nodiscard]] bool contains(Address address) const noexcept;

    [[nodiscard]] auto findExport(std::string_view symbolName) const -> Expected<Address>;
    [[nodiscard]] auto enumerateExports() const -> std::vector<std::pair<std::string, Address>>;
    [[nodiscard]] auto findSection(std::string_view sectionName) const -> Expected<std::pair<Address, std::size_t>>;

private:
    std::string mName;
    Address     mBase{};
    std::size_t mSize{};
};

struct SignatureElement {
    std::byte value{};
    std::byte mask{};

    [[nodiscard]] static constexpr auto byte(const std::byte value) noexcept -> SignatureElement {
        return {value, std::byte{0xFF}};
    }

    [[nodiscard]] static constexpr auto wildcard() noexcept -> SignatureElement {
        return {};
    }

    [[nodiscard]] constexpr bool isWildcard() const noexcept {
        return mask == std::byte{0};
    }

    [[nodiscard]] constexpr bool isConcrete() const noexcept {
        return mask == std::byte{0xFF};
    }

    [[nodiscard]] constexpr bool matches(const std::byte valueToCheck) const noexcept {
        return (valueToCheck & mask) == value;
    }
};

class CloverNT_API Signature {
public:
    Signature() = default;
    explicit Signature(std::vector<SignatureElement> elements);

    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] auto size() const noexcept -> std::size_t;
    [[nodiscard]] auto elements() const noexcept -> std::span<SignatureElement const>;
    [[nodiscard]] auto toString() const -> std::string;

private:
    std::vector<SignatureElement> mElements;
};

enum class ScanAlignment : std::uint8_t {
    X1  = 1,
    X16 = 16,
};

enum class ScanHint : std::uint64_t {
    None   = 0,
    X86_64 = 1 << 0,
    Pair0  = 1 << 1,
};

constexpr auto operator|(const ScanHint lhs, const ScanHint rhs) noexcept -> ScanHint {
    return static_cast<ScanHint>(std::to_underlying(lhs) | std::to_underlying(rhs));
}

constexpr auto operator&(const ScanHint lhs, const ScanHint rhs) noexcept -> ScanHint {
    return static_cast<ScanHint>(std::to_underlying(lhs) & std::to_underlying(rhs));
}

struct ScanOptions {
    ScanAlignment alignment{ScanAlignment::X1};
    ScanHint      hints{ScanHint::None};
};

class ScanResult {
public:
    constexpr ScanResult() noexcept = default;
    explicit constexpr ScanResult(const Address address) noexcept : mAddress(address) {}

    [[nodiscard]] constexpr bool hasResult() const noexcept {
        return mAddress != 0;
    }

    [[nodiscard]] explicit constexpr operator bool() const noexcept {
        return hasResult();
    }

    [[nodiscard]] constexpr auto get() const noexcept -> Address {
        return mAddress;
    }

    template <std::integral Int>
    [[nodiscard]] auto read(const std::size_t offset = 0) const noexcept -> Int {
        Int value{};
        std::memcpy(&value, reinterpret_cast<void const*>(mAddress + offset), sizeof(Int));
        return value;
    }

    [[nodiscard]] auto rel(const std::size_t offset, const std::size_t remaining = 0) const noexcept -> Address {
        const auto displacement = read<std::int32_t>(offset);
        return mAddress + offset + sizeof(std::int32_t) + remaining + static_cast<Address>(displacement);
    }

private:
    Address mAddress{};
};

struct ResourceOptions {
    std::weak_ptr<Plugin::Plugin> owner;
};

class CloverNT_API PatchHandle {
public:
    PatchHandle() = default;
    explicit PatchHandle(ResourceId id) noexcept;
    ~PatchHandle();

    PatchHandle(PatchHandle const&)            = delete;
    PatchHandle& operator=(PatchHandle const&) = delete;

    PatchHandle(PatchHandle&& other) noexcept;
    auto operator=(PatchHandle&& other) noexcept -> PatchHandle&;

    [[nodiscard]] explicit operator bool() const noexcept;
    [[nodiscard]] auto     id() const noexcept -> ResourceId;
    [[nodiscard]] bool     isApplied() const;

    [[nodiscard]] auto apply() const -> Expected<void>;
    [[nodiscard]] auto restore() const -> Expected<void>;
    [[nodiscard]] auto remove() -> Expected<void>;

private:
    ResourceId mId{};
};

[[nodiscard]] CloverNT_API auto findModule(std::string_view moduleName = {}) -> Expected<Module>;
[[nodiscard]] CloverNT_API auto enumerateModules() -> std::vector<Module>;

[[nodiscard]] CloverNT_API auto readMemory(void* dest, Address source, std::size_t size) -> Expected<void>;
[[nodiscard]] CloverNT_API auto writeMemory(Address dest, void const* source, std::size_t size) -> Expected<void>;
[[nodiscard]] CloverNT_API auto setProtection(Address address, std::size_t size, MemoryProtection protection)
        -> Expected<MemoryProtection>;
[[nodiscard]] CloverNT_API auto setProtectionRaw(Address address, std::size_t size, MemoryProtection protection)
        -> Expected<void>;
[[nodiscard]] CloverNT_API auto queryProtection(Address address) -> Expected<MemoryProtection>;
[[nodiscard]] CloverNT_API bool isReadable(Address address, std::size_t size = 1);
[[nodiscard]] CloverNT_API bool isWritable(Address address, std::size_t size = 1);

template <class T>
    requires std::is_trivially_copyable_v<T>
[[nodiscard]] auto read(const Address address) -> Expected<T> {
    T value{};
    if (auto result = readMemory(&value, address, sizeof(T)); !result) {
        return unexpected(result.error());
    }
    return value;
}

template <class T>
    requires std::is_trivially_copyable_v<T>
[[nodiscard]] auto write(const Address address, T const& value) -> Expected<void> {
    return writeMemory(address, &value, sizeof(T));
}

[[nodiscard]] CloverNT_API auto readBytes(Address address, std::size_t size) -> Expected<std::vector<std::uint8_t>>;
[[nodiscard]] CloverNT_API auto writeBytes(Address address, std::span<std::uint8_t const> bytes) -> Expected<void>;

[[nodiscard]] CloverNT_API auto parseSignature(std::string_view pattern) -> Expected<Signature>;
[[nodiscard]] CloverNT_API auto
findFirst(std::string_view moduleName, std::string_view pattern, ScanOptions options = {}) -> ScanResult;
[[nodiscard]] CloverNT_API auto findFirst(Module const& module, std::string_view pattern, ScanOptions options = {})
        -> ScanResult;
[[nodiscard]] CloverNT_API auto findAll(std::string_view moduleName, std::string_view pattern, ScanOptions options = {})
        -> std::vector<ScanResult>;
[[nodiscard]] CloverNT_API auto findAll(Module const& module, std::string_view pattern, ScanOptions options = {})
        -> std::vector<ScanResult>;
[[nodiscard]] CloverNT_API auto
findFirst(std::string_view moduleName, Signature const& signature, ScanOptions options = {}) -> ScanResult;
[[nodiscard]] CloverNT_API auto findFirst(Module const& module, Signature const& signature, ScanOptions options = {})
        -> ScanResult;
[[nodiscard]] CloverNT_API auto
findAll(std::string_view moduleName, Signature const& signature, ScanOptions options = {}) -> std::vector<ScanResult>;
[[nodiscard]] CloverNT_API auto findAll(Module const& module, Signature const& signature, ScanOptions options = {})
        -> std::vector<ScanResult>;
[[nodiscard]] CloverNT_API auto findFirstInSection(std::string_view moduleName,
                                                   std::string_view sectionName,
                                                   Signature const& signature,
                                                   ScanOptions      options = {}) -> ScanResult;
[[nodiscard]] CloverNT_API auto findAllInSection(std::string_view moduleName,
                                                 std::string_view sectionName,
                                                 Signature const& signature,
                                                 ScanOptions      options = {}) -> std::vector<ScanResult>;

[[nodiscard]] CloverNT_API auto createPatch(Address                   address,
                                            std::vector<std::uint8_t> bytes,
                                            const ResourceOptions&    options = {}) -> Expected<PatchHandle>;
[[nodiscard]] CloverNT_API auto createNopPatch(Address address, std::size_t size, const ResourceOptions& options = {})
        -> Expected<PatchHandle>;

namespace Detail {

    class CloverNT_API ResourcePayload {
    public:
        virtual ~ResourcePayload();
    };

    [[nodiscard]] CloverNT_API auto createInlineHook(Address                              target,
                                                     Address                              detour,
                                                     int                                  priority,
                                                     void**                               originalSlot,
                                                     const std::weak_ptr<Plugin::Plugin>& owner,
                                                     std::shared_ptr<ResourcePayload> payload) -> Expected<ResourceId>;
    [[nodiscard]] CloverNT_API auto createImportHook(std::string_view                     moduleName,
                                                     std::string_view                     importModule,
                                                     std::string_view                     functionName,
                                                     Address                              detour,
                                                     void**                               originalSlot,
                                                     const std::weak_ptr<Plugin::Plugin>& owner,
                                                     std::shared_ptr<ResourcePayload> payload) -> Expected<ResourceId>;

    [[nodiscard]] CloverNT_API auto enableResource(ResourceId id) -> Expected<void>;
    [[nodiscard]] CloverNT_API auto disableResource(ResourceId id) -> Expected<void>;
    [[nodiscard]] CloverNT_API auto removeResource(ResourceId id) -> Expected<void>;
    [[nodiscard]] CloverNT_API bool isResourceEnabled(ResourceId id);

} // namespace Detail

} // namespace CloverNT::Memory
