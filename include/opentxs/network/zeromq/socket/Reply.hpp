// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_REPLY_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_REPLY_HPP

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
class Reply;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQReplySocket = Pimpl<network::zeromq::socket::Reply>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Reply : virtual public curve::Server
{
public:
    ~Reply() override = default;

protected:
    Reply() noexcept = default;

private:
    friend OTZMQReplySocket;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const noexcept -> Reply* = 0;
#ifdef _WIN32
private:
#endif

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
