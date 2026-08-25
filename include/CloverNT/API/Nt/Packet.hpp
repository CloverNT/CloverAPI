#pragma once

#include <CloverNT/API/Macros.hpp>

#include <array>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace CloverNT::Nt {

/// Raw byte payload.
using Bytes = std::vector<std::uint8_t>;

/// Encryption applied to an SSO packet body.
enum class Encryption : std::uint8_t {
    None    = 0, ///< Plaintext body.
    D2Key   = 1, ///< Encrypted with the session d2 key.
    ZeroKey = 2, ///< Encrypted with the zero key (login handshake).
};

/// A single SSO packet.
///
/// For received packets `body` is the decrypted payload; for outgoing packets it
/// is the plaintext payload to send.
struct Packet {
    std::uint32_t seq{};
    std::string   command;
    std::string   uin;
    Encryption    encryption{Encryption::None};
    Bytes         body;
};

/// A plaintext o3 secure-channel packet.
struct O3Packet {
    std::string command;
    Bytes       body;
};

/// The captured session credentials.
struct Credential {
    std::string                  uin;
    Bytes                        a2;
    Bytes                        d2;
    std::array<std::uint8_t, 16> d2Key{};
};

/// Default timeout for the blocking wait helpers.
inline constexpr std::chrono::milliseconds kDefaultTimeout{5000};

/// Whether the low-level direct send path has been discovered from wrapper.node.
[[nodiscard]] CloverNT_API auto ready() -> bool;

/// The most recently captured session credentials, if any.
[[nodiscard]] CloverNT_API auto credential() -> std::optional<Credential>;

/// Send a packet (fire-and-forget). Returns the captured sequence id when the
/// interceptor was able to observe it; std::nullopt if the send path is
/// unavailable.
[[nodiscard]] CloverNT_API auto send(std::string_view command, std::span<const std::uint8_t> body)
        -> std::optional<std::uint32_t>;

/// Send a packet and wait for its response.
[[nodiscard]] CloverNT_API auto call(std::string_view              command,
                                     std::span<const std::uint8_t> body,
                                     std::chrono::milliseconds     timeout = kDefaultTimeout) -> std::optional<Packet>;

/// Wait for the next received packet with the given service command.
[[nodiscard]] CloverNT_API auto receive(std::string_view command, std::chrono::milliseconds timeout = kDefaultTimeout)
        -> std::optional<Packet>;

/// Wait for the next received packet with the given sequence id.
[[nodiscard]] CloverNT_API auto receive(std::uint32_t seq, std::chrono::milliseconds timeout = kDefaultTimeout)
        -> std::optional<Packet>;

} // namespace CloverNT::Nt
