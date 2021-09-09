// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_SUBSCRIBE_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_SUBSCRIBE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Subscribe;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQSubscribeSocket = Pimpl<network::zeromq::socket::Subscribe>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Subscribe : virtual public curve::Client
{
public:
    virtual auto SetSocksProxy(const std::string& proxy) const noexcept
        -> bool = 0;

    ~Subscribe() override = default;

protected:
    Subscribe() noexcept = default;

private:
    friend OTZMQSubscribeSocket;

    virtual auto clone() const noexcept -> Subscribe* = 0;

    Subscribe(const Subscribe&) = delete;
    Subscribe(Subscribe&&) = delete;
    auto operator=(const Subscribe&) -> Subscribe& = delete;
    auto operator=(Subscribe&&) -> Subscribe& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
