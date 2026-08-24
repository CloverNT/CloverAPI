# CloverAPI

The developer-facing SDK for building **CloverNT** plugins, plus the standalone,
dependency-free API layer it is built on.

This repository is a **generated mirror** of CloverNT's `include/CloverNT/API`,
`src/API` and the cmake helpers. Do not edit it directly — it is published from the
CloverNT repository via `scripts/push-api.ps1`. The CloverCore import library lives in
its own repo (`CloverNT/CloverCore`); CloverAPI requires it and pulls it in for you, so
plugins just link **`Clover::Core`**.

## What's here

| Target / file                | Purpose                                                                   |
|------------------------------|---------------------------------------------------------------------------|
| `Clover::Core`               | What plugins link: the API headers + the CloverCore import library (pulled from the CloverCore repo's Releases). |
| `CloverAPI` (static lib)     | The dependency-free API layer (Logger, Utils). For hosts *without* the runtime. |
| `cmake/use-libcxx.cmake`     | Downloads the matching libc++ (ABI `__Cr`) and exposes `target_use_libcxx()`. |
| `cmake/qqnt-sdk.cmake`       | Downloads the QQNT SDK and exposes `QQNT::QQNT` (V8 / Node / libuv headers + libs). |
| `cmake/clover-core.cmake`    | Used *internally* by this package to fetch the import lib and define `Clover::Core`. |

## Native (C++) plugin usage

A native plugin is a shared library that exports `clovernt_plugin_load`. It must be
compiled with **Clang** and the same libc++ ABI (`__Cr`) the runtime uses, so its
`std::` types match across the DLL boundary.

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_plugin LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(CloverAPI
        GIT_REPOSITORY https://github.com/CloverNT/CloverAPI
        GIT_TAG main)
FetchContent_MakeAvailable(CloverAPI)          # -> Clover::Core (API headers + core import lib)

# Pull the SDK's cmake helpers and fetch the toolchain deps remotely.
list(APPEND CMAKE_MODULE_PATH "${cloverapi_SOURCE_DIR}/cmake")
set(LIBCXX_GITHUB_REPO   CloverNT/libcxx-build)
set(LIBCXX_ABI_NAMESPACE __Cr)
set(QQNT_SDK_REPO        CloverNT/qqnt-sdk)
include(use-libcxx)                            # -> target_use_libcxx()
include(qqnt-sdk)                              # -> QQNT::QQNT

add_library(my_plugin SHARED main.cpp)
target_link_libraries(my_plugin PRIVATE Clover::Core)
target_use_libcxx(my_plugin)                   # match the runtime's libc++ ABI
```

> To pin a specific core build, `set(CLOVER_CORE_VERSION <tag>)` *before*
> `FetchContent_MakeAvailable(CloverAPI)` (defaults to `latest`).

```cpp
#include <CloverNT/API/Logger.hpp>
#include <CloverNT/API/Plugin/NativePlugin.hpp>
#include <CloverNT/API/Plugin/RegisterHelper.hpp>

struct MyPlugin {
    bool load(CloverNT::Plugin::NativePlugin& self) { /* ... */ return true; }
};
CloverNT_REGISTER_PLUGIN(MyPlugin)
```

## API-only usage (no runtime)

```cmake
FetchContent_MakeAvailable(CloverAPI)
target_link_libraries(your_target PRIVATE CloverAPI)
```

```cpp
#include <CloverNT/API/Logger.hpp>
#include <CloverNT/API/Expected.hpp>
```

Requires a C++23 toolchain. `CloverAPI` pulls in no third-party dependencies; it
links `dl` on non-Windows platforms and nothing extra on Windows.
