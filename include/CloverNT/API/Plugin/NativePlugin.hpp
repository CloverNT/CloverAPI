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
    using Callback = bool (*)(NativePlugin&); // load entry (the extern "C" clovernt_plugin_load symbol)
    using UnloadFn = bool (*)();              // unload handler the plugin registers during load()

    NativePlugin(Manifest manifest, std::filesystem::path directory, std::filesystem::path entryPath);
    ~NativePlugin() override;

    [[nodiscard]] auto  directory() const noexcept -> std::filesystem::path const&;
    [[nodiscard]] auto  entryPath() const noexcept -> std::filesystem::path const&;
    [[nodiscard]] void* nativeHandle() const noexcept;

    [[nodiscard]] bool hasUnloadHandler() const noexcept {
        return mUnloadFn != nullptr;
    }
    [[nodiscard]] bool invokeUnloadHandler() const {
        return mUnloadFn != nullptr ? mUnloadFn() : true;
    }

private:
    std::filesystem::path mDirectory;
    std::filesystem::path mEntryPath;
    Utils::DynamicLibrary mLibrary;
    Callback              mLoadCallback{};
    UnloadFn              mUnloadFn{};

    friend void setUnloadHandler(NativePlugin& plugin, UnloadFn fn) noexcept;
    friend class PluginManager;
    friend class Core::Modules::PluginRuntime;
};

// Registers a plugin's unload handler. Kept as a free (non-member) inline function on purpose: NativePlugin
// is CloverNT_API (dllimport for plugins), and an inline *member* would be emitted as a dllimport reference
// under /Ob0 (Debug), forcing every plugin/fixture to link the core. This friend free function is a plain
// inline symbol instead, so plugins and test fixtures can register a handler without linking the core.
inline void setUnloadHandler(NativePlugin& plugin, NativePlugin::UnloadFn fn) noexcept {
    plugin.mUnloadFn = fn;
}

using NativePluginPtr = std::shared_ptr<NativePlugin>;

} // namespace CloverNT::Plugin
