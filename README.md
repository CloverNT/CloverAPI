# CloverAPI

The standalone, dependency-free API layer of **CloverNT**.

This repository is a **generated mirror** of CloverNT's `include/CloverNT/API` and `src/API`. Do not edit it
directly — changes are published from the CloverNT repository via `scripts/push-api.ps1`.

## Usage

```cmake
include(FetchContent)
FetchContent_Declare(
        CloverAPI
        GIT_REPOSITORY https://github.com/CloverNT/CloverAPI
        GIT_TAG main
)
FetchContent_MakeAvailable(CloverAPI)

target_link_libraries(your_target PRIVATE CloverAPI)
```

```cpp
#include <CloverNT/API/Logger.hpp>
#include <CloverNT/API/Expected.hpp>
```

Requires a C++23 toolchain. The library pulls in no third-party dependencies; it links `dl` on non-Windows
platforms and nothing extra on Windows.
