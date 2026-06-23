#include <CloverNT/API/Utils/System.hpp>

#include <Windows.h>

namespace CloverNT::Utils::System {

auto GetCurrentPlatform() noexcept -> Platform {
    return Platform::Windows;
}

auto GetCurrentPlatformName() noexcept -> std::string_view {
    return "windows";
}

auto GetCurrentTimeWithMs() noexcept -> TimeWithMs {
    SYSTEMTIME localTime{};
    GetLocalTime(&localTime);
    return TimeWithMs{
            .hour        = localTime.wHour,
            .minute      = localTime.wMinute,
            .second      = localTime.wSecond,
            .millisecond = localTime.wMilliseconds,
    };
}

auto GetModuleDirectory(const ModuleHandle module) -> std::filesystem::path {
    std::wstring buffer;
    buffer.resize(32768);
    const auto size = GetModuleFileNameW(module, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (size == 0 || size >= buffer.size()) {
        return std::filesystem::current_path();
    }
    buffer.resize(size);
    return std::filesystem::path(buffer).parent_path();
}

} // namespace CloverNT::Utils::System
