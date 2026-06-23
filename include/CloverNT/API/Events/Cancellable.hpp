#pragma once

#include <CloverNT/API/Macros.hpp>

namespace CloverNT::Event {

class CloverNT_API Cancellable {
public:
    [[nodiscard]] bool isCancelled() const noexcept;
    void               setCancelled(bool cancelled = true) noexcept;

private:
    bool mCancelled{false};
};

} // namespace CloverNT::Event
