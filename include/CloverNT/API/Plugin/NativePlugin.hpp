#pragma once

#include <filesystem>

#include <CloverNT/API/Plugin/Plugin.hpp>
#include <CloverNT/API/Utils/DynamicLibrary.hpp>

namespace CloverNT::Core::Modules {
class PluginRuntime;
}

namespace CloverNT::Plugin {

class CloverNT_API NativePlugin : public Plugin {
public:
    using Callback = bool (*)(NativePlugin&);

    NativePlugin(Manifest manifest, std::filesystem::path directory, std::filesystem::path entryPath);
    ~NativePlugin() override;

    [[nodiscard]] auto  directory() const noexcept -> std::filesystem::path const&;
    [[nodiscard]] auto  entryPath() const noexcept -> std::filesystem::path const&;
    [[nodiscard]] void* nativeHandle() const noexcept;

private:
    std::filesystem::path mDirectory;
    std::filesystem::path mEntryPath;
    Utils::DynamicLibrary mLibrary;
    Callback              mLoadCallback{};
    Callback              mUnloadCallback{};

    friend class PluginManager;
    friend class Core::Modules::PluginRuntime;
};

using NativePluginPtr = std::shared_ptr<NativePlugin>;

} // namespace CloverNT::Plugin
