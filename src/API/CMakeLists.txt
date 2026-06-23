add_library(CloverAPI STATIC
        Logger.cpp
        Utils/DynamicLibrary.cpp
        Utils/System.cpp
        $<$<PLATFORM_ID:Windows>:Utils/DynamicLibrary.Windows.cpp>
        $<$<PLATFORM_ID:Linux>:Utils/DynamicLibrary.Linux.cpp>
        $<$<PLATFORM_ID:Windows>:Utils/System.Windows.cpp>
        $<$<PLATFORM_ID:Linux>:Utils/System.Linux.cpp>
)

target_include_directories(CloverAPI PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(CloverAPI
        PUBLIC
        CloverNT_API_STATIC
        $<$<CONFIG:Debug>:CloverNT_DEBUG>
        ${CloverNT_SYSTEM_MACRO}
        ${CloverNT_ARCH_MACRO}
        PRIVATE
        CloverNT_API_EXPORTS
)

if (NOT WIN32)
    target_link_libraries(CloverAPI PUBLIC dl)
endif ()
