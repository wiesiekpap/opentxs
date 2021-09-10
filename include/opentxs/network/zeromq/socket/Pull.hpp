// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_PULL_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_PULL_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/curve/Server.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Pull;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPullSocket = Pimpl<network::zeromq::socket::Pull>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Pull : virtual public curve::Server
{
public:
    ~Pull() override = default;

protected:
    Pull() noexcept = default;

private:
    friend OTZMQPullSocket;

    virtual auto clone() const noexcept -> Pull* = 0;

    Pull(const Pull&) = delete;
    Pull(Pull&&) = delete;
    auto operator=(const Pull&) -> Pull& = delete;
    auto operator=(Pull&&) -> Pull& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
