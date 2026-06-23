#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>

#include <CloverNT/API/Macros.hpp>

namespace CloverNT::Utils::System {

enum class Platform {
    Windows,
    Linux,
};

struct TimeWithMs {
    std::uint16_t hour{};
    std::uint16_t minute{};
    std::uint16_t second{};
    std::uint16_t millisecond{};
};

[[nodiscard]] CloverNT_API Platform GetCurrentPlatform() noexcept;

[[nodiscard]] CloverNT_API std::string_view GetCurrentPlatformName() noexcept;

[[nodiscard]] CloverNT_API TimeWithMs GetCurrentTimeWithMs() noexcept;

[[nodiscard]] CloverNT_API std::filesystem::path GetModuleDirectory(ModuleHandle module);

[[nodiscard]] CloverNT_API bool IsPathInside(std::filesystem::path const& child, std::filesystem::path const& parent);

} // namespace CloverNT::Utils::System
