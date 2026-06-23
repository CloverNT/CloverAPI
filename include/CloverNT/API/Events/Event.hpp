#pragma once

#include <string_view>
#include <typeinfo>

#include <CloverNT/API/Macros.hpp>

namespace CloverNT::Event {

class CloverNT_API Event {
public:
    virtual ~Event();
};

template <class T>
[[nodiscard]] auto getEventName() noexcept -> std::string_view {
    return typeid(T).name();
}

} // namespace CloverNT::Event
