// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/asio/Endpoint.hpp"
// IWYU pragma: no_include "opentxs/network/asio/Socket.hpp"

#ifndef OPENTXS_API_NETWORK_ASIO_HPP
#define OPENTXS_API_NETWORK_ASIO_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string_view>
#include <vector>

namespace opentxs
{
namespace network
{
namespace asio
{
class Endpoint;
class Socket;
}  // namespace asio

namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace network
{
class Asio
{
public:
    using Endpoint = opentxs::network::asio::Endpoint;
    using Socket = opentxs::network::asio::Socket;
    using Resolved = std::vector<Endpoint>;

    OPENTXS_EXPORT auto MakeSocket(const Endpoint& endpoint) const noexcept
        -> Socket;
    OPENTXS_EXPORT auto NotificationEndpoint() const noexcept -> const char*;
    OPENTXS_EXPORT auto Resolve(std::string_view server, std::uint16_t port)
        const noexcept -> Resolved;

    auto Init() noexcept -> void;
    auto Shutdown() noexcept -> void;

    Asio(const opentxs::network::zeromq::Context& zmq) noexcept;

    ~Asio();

private:
    struct Imp;

    Imp* imp_;

    Asio() = delete;
    Asio(const Asio&) = delete;
    Asio(Asio&&) = delete;
    Asio& operator=(const Asio&) = delete;
    Asio& operator=(Asio&&) = delete;
};
}  // namespace network
}  // namespace api
}  // namespace opentxs
#endif
