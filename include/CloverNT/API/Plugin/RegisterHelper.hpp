#pragma once

#include <concepts>
#include <type_traits>

#include <CloverNT/API/Macros.hpp>
#include <CloverNT/API/Plugin/NativePlugin.hpp>

namespace CloverNT::Plugin::Detail {

template <class T>
concept LoadablePlugin = requires(T value, NativePlugin& self) {
    { value.load(self) } -> std::same_as<bool>;
};

template <class T>
concept UnloadablePlugin = requires(T value, NativePlugin& self) {
    { value.unload(self) } -> std::same_as<bool>;
};

} // namespace CloverNT::Plugin::Detail

#define CloverNT_REGISTER_PLUGIN(TYPE)                                                                                 \
    namespace {                                                                                                        \
        TYPE& clovernt_plugin_instance() {                                                                             \
            static TYPE instance{};                                                                                    \
            return instance;                                                                                           \
        }                                                                                                              \
    }                                                                                                                  \
    extern "C" CloverNT_EXPORT bool clovernt_plugin_load(::CloverNT::Plugin::NativePlugin& self) {                     \
        if constexpr (::CloverNT::Plugin::Detail::LoadablePlugin<TYPE>) {                                              \
            return clovernt_plugin_instance().load(self);                                                              \
        } else {                                                                                                       \
            (void) self;                                                                                               \
            return true;                                                                                               \
        }                                                                                                              \
    }

#define CloverNT_REGISTER_UNLOADABLE_PLUGIN(TYPE)                                                                      \
    namespace {                                                                                                        \
        TYPE& clovernt_plugin_instance() {                                                                             \
            static TYPE instance{};                                                                                    \
            return instance;                                                                                           \
        }                                                                                                              \
    }                                                                                                                  \
    extern "C" CloverNT_EXPORT bool clovernt_plugin_load(::CloverNT::Plugin::NativePlugin& self) {                     \
        if constexpr (::CloverNT::Plugin::Detail::LoadablePlugin<TYPE>) {                                              \
            return clovernt_plugin_instance().load(self);                                                              \
        } else {                                                                                                       \
            (void) self;                                                                                               \
            return true;                                                                                               \
        }                                                                                                              \
    }                                                                                                                  \
    extern "C" CloverNT_EXPORT bool clovernt_plugin_unload(::CloverNT::Plugin::NativePlugin& self) {                   \
        static_assert(::CloverNT::Plugin::Detail::UnloadablePlugin<TYPE>,                                              \
                      "CloverNT_REGISTER_UNLOADABLE_PLUGIN requires bool unload(NativePlugin&)");                      \
        return clovernt_plugin_instance().unload(self);                                                                \
    }
