// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_ROUTER_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_ROUTER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/network/zeromq/curve/Server.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Router;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQRouterSocket = Pimpl<network::zeromq::socket::Router>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Router : virtual public curve::Server,
                              virtual public curve::Client,
                              virtual public Sender
{
public:
    virtual auto SetSocksProxy(const std::string& proxy) const noexcept
        -> bool = 0;

    ~Router() override = default;

protected:
    Router() noexcept = default;

private:
    friend OTZMQRouterSocket;

    virtual auto clone() const noexcept -> Router* = 0;

    Router(const Router&) = delete;
    Router(Router&&) = delete;
    auto operator=(const Router&) -> Router& = delete;
    auto operator=(Router&&) -> Router& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
