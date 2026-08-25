# CloverAPI.cmake — bootstrap the CloverNT SDK (the `Clover::Core` link target and
# the `CloverAPI` static lib). include() this from a plugin project. With no
# configuration it fetches CloverNT/CloverAPI. To override:
#   CLOVER_API_SOURCE_DIR  use a local SDK / CloverAPI checkout instead of fetching
#   CLOVER_API_TAG         pin a released CloverAPI git tag (default: main)
#   CLOVER_API_REPOSITORY  fetch from a fork/mirror instead of the canonical repo
if (TARGET Clover::Core)
    return()
endif ()

if (CLOVER_API_SOURCE_DIR)
    get_filename_component(_clover_api_dir "${CLOVER_API_SOURCE_DIR}" ABSOLUTE)
    if (NOT EXISTS "${_clover_api_dir}/CMakeLists.txt")
        message(FATAL_ERROR "CloverAPI: CLOVER_API_SOURCE_DIR has no CMakeLists.txt: ${_clover_api_dir}")
    endif ()
    message(STATUS "CloverAPI: using local SDK at ${_clover_api_dir}")
    add_subdirectory("${_clover_api_dir}" cloverapi)
    set(_clover_api_cmake "${_clover_api_dir}/cmake")
else ()
    include(FetchContent)
    if (NOT DEFINED CLOVER_API_REPOSITORY)
        set(CLOVER_API_REPOSITORY "https://github.com/CloverNT/CloverAPI")
    endif ()
    if (NOT DEFINED CLOVER_API_TAG)
        set(CLOVER_API_TAG "main")
    endif ()
    message(STATUS "CloverAPI: fetching ${CLOVER_API_REPOSITORY}@${CLOVER_API_TAG}")
    FetchContent_Declare(CloverAPI
            GIT_REPOSITORY "${CLOVER_API_REPOSITORY}"
            GIT_TAG "${CLOVER_API_TAG}")
    FetchContent_MakeAvailable(CloverAPI)
    set(_clover_api_cmake "${cloverapi_SOURCE_DIR}/cmake")
endif ()

# Expose the SDK's cmake helpers (Libc++, QQNTSdk) on the module path so the
# consumer can include() them regardless of how the SDK was resolved.
list(APPEND CMAKE_MODULE_PATH "${_clover_api_cmake}")
set(CLOVER_API_CMAKE_DIR "${_clover_api_cmake}")
