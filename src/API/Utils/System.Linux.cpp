#include <CloverNT/API/Utils/System.hpp>

#include <ctime>
#include <system_error>

#include <dlfcn.h>

namespace CloverNT::Utils::System {

Platform GetCurrentPlatform() noexcept {
    return Platform::Linux;
}

std::string_view GetCurrentPlatformName() noexcept {
    return "linux";
}

TimeWithMs GetCurrentTimeWithMs() noexcept {
    timespec    now{};
    std::time_t seconds      = 0;
    auto        milliseconds = 0;

    if (clock_gettime(CLOCK_REALTIME, &now) == 0) {
        seconds      = static_cast<std::time_t>(now.tv_sec);
        milliseconds = static_cast<int>(now.tv_nsec / 1'000'000);
    } else {
        seconds = std::time(nullptr);
    }

    std::tm localTime{};
    if (seconds == static_cast<std::time_t>(-1) || localtime_r(&seconds, &localTime) == nullptr) {
        return {};
    }

    return TimeWithMs{
            .hour        = static_cast<std::uint16_t>(localTime.tm_hour),
            .minute      = static_cast<std::uint16_t>(localTime.tm_min),
            .second      = static_cast<std::uint16_t>(localTime.tm_sec),
            .millisecond = static_cast<std::uint16_t>(milliseconds),
    };
}

std::filesystem::path GetModuleDirectory(ModuleHandle module) {
    Dl_info info{};
    auto*   address = module != nullptr ? module : reinterpret_cast<void*>(&GetModuleDirectory);
    if (::dladdr(address, &info) != 0 && info.dli_fname != nullptr) {
        std::error_code ec;
        auto            absolute = std::filesystem::absolute(std::filesystem::path(info.dli_fname), ec);
        if (!ec) {
            return absolute.parent_path();
        }
    }

    std::error_code ec;
    auto            current = std::filesystem::current_path(ec);
    return ec ? std::filesystem::path{"."} : current;
}

} // namespace CloverNT::Utils::System
