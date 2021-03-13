// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_ROUTER_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_ROUTER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"
#include "opentxs/network/zeromq/curve/Server.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Router>::Pimpl(opentxs::network::zeromq::socket::Router const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Router>::operator opentxs::network::zeromq::socket::Router&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::socket::Router>::operator const opentxs::network::zeromq::socket::Router &;
%rename(assign) operator=(const opentxs::network::zeromq::socket::Router&);
%rename(ZMQRouter) opentxs::network::zeromq::socket::Router;
%template(OTZMQRouterSocket) opentxs::Pimpl<opentxs::network::zeromq::socket::Router>;
// clang-format on
#endif  // SWIG

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
class Router : virtual public curve::Server,
               virtual public curve::Client,
               virtual public Sender
{
public:
    OPENTXS_EXPORT virtual bool SetSocksProxy(
        const std::string& proxy) const noexcept = 0;

    OPENTXS_EXPORT ~Router() override = default;

protected:
    Router() noexcept = default;

private:
    friend OTZMQRouterSocket;

    virtual Router* clone() const noexcept = 0;

    Router(const Router&) = delete;
    Router(Router&&) = delete;
    Router& operator=(const Router&) = delete;
    Router& operator=(Router&&) = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
