# CloverAPI

The developer-facing SDK for building **CloverNT** plugins, plus the standalone, dependency-free API layer it is built
on.

This repository is a **generated mirror** of CloverNT's `include/CloverNT/API`,
`src/API`, the cmake helpers and the prebuilt CloverCore import libraries. Do not edit it directly — it is published
from the CloverNT repository CI. The whole checkout **is** the SDK: the import library ships under `lib/<os-arch>/`, so
plugins just link **`Clover::Core`** with nothing to download. Pin a version by checking out (or `FetchContent`-ing) a
release git tag.

## What's here

| Target / file            | Purpose                                                                                        |
|--------------------------|------------------------------------------------------------------------------------------------|
| `Clover::Core`           | What plugins link: the API headers + the bundled CloverCore import library (`lib/<os-arch>/`). |
| `CloverAPI` (static lib) | The dependency-free API layer (Logger, Utils). For hosts *without* the runtime.                |
| `lib/<os-arch>/`         | The prebuilt CloverCore import libraries shipped with the SDK, one per platform slot.          |
| `cmake/Libc++.cmake`     | Downloads the matching libc++ (ABI `__Cr`) and exposes `target_use_libcxx()`.                  |
| `cmake/QQNTSdk.cmake`    | Downloads the QQNT SDK and exposes `QQNT::QQNT` (V8 / Node / libuv headers + libs).            |
| `cmake/CloverCore.cmake` | Resolves the bundled import lib for your platform and defines `Clover::Core`.                  |
| `cmake/CloverAPI.cmake`  | Drop-in bootstrap: fetches (or uses a local) SDK and puts the helpers on the module path.      |

## Native (C++) plugin usage

A native plugin is a shared library that exports `clovernt_plugin_load`. It must be compiled with **Clang** and the same
libc++ ABI (`__Cr`) the runtime uses, so its
`std::` types match across the DLL boundary.

```cmake
cmake_minimum_required(VERSION 3.25)
project(my_plugin LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

# Zero-config bootstrap: fetches CloverNT/CloverAPI (the SDK). Drop CloverAPI.cmake
# into your project, or download it from the SDK repo as below.
file(DOWNLOAD
        https://raw.githubusercontent.com/CloverNT/CloverAPI/main/cmake/CloverAPI.cmake
        "${CMAKE_BINARY_DIR}/CloverAPI.cmake")
include("${CMAKE_BINARY_DIR}/CloverAPI.cmake")  # -> Clover::Core (+ SDK cmake helpers on module path)

set(LIBCXX_GITHUB_REPO CloverNT/libcxx-build)
set(LIBCXX_ABI_NAMESPACE __Cr)
set(QQNT_SDK_REPO CloverNT/qqnt-sdk)
include(Libc++)                             # -> target_use_libcxx()
include(QQNTSdk)                               # -> QQNT::QQNT

add_library(my_plugin SHARED main.cpp)
target_link_libraries(my_plugin PRIVATE Clover::Core)
target_use_libcxx(my_plugin)                   # match the runtime's libc++ ABI
```

> Pin a version with `-DCLOVER_API_TAG=<release tag>`. To use an SDK unpacked elsewhere, set
> `-DCLOVER_API_SOURCE_DIR=<path>` (the bootstrap `add_subdirectory`s it instead of fetching).

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

Requires a C++23 toolchain. `CloverAPI` pulls in no third-party dependencies; it links `dl` on non-Windows platforms and
nothing extra on Windows.
