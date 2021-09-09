// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_SOCKET_PAIR_HPP
#define OPENTXS_NETWORK_ZEROMQ_SOCKET_PAIR_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Pair;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQPairSocket = Pimpl<network::zeromq::socket::Pair>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class OPENTXS_EXPORT Pair : virtual public socket::Socket, virtual public Sender
{
public:
    virtual auto Endpoint() const noexcept -> const std::string& = 0;

    ~Pair() override = default;

protected:
    Pair() noexcept = default;

private:
    friend OTZMQPairSocket;

    virtual auto clone() const noexcept -> Pair* = 0;

    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    auto operator=(const Pair&) -> Pair& = delete;
    auto operator=(Pair&&) -> Pair& = delete;
};
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
