// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ostream>

#include "network/zeromq/socket/Bidirectional.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Container.hpp"

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

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket::implementation
{
class Pair final : public Bidirectional<zeromq::socket::Pair>
{
public:
    auto Endpoint() const noexcept -> const UnallocatedCString& final;
    auto Start(const UnallocatedCString& endpoint) const noexcept -> bool final
    {
        return false;
    }

    Pair(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const UnallocatedCString& endpoint,
        const Socket::Direction direction,
        const bool startThread) noexcept;
    Pair(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const bool startThread = true) noexcept;
    Pair(
        const zeromq::ListenCallback& callback,
        const zeromq::socket::Pair& peer,
        const bool startThread = true) noexcept;
    Pair(
        const zeromq::Context& context,
        const zeromq::ListenCallback& callback,
        const UnallocatedCString& endpoint) noexcept;

    ~Pair() final;

private:
    const ListenCallback& callback_;
    const UnallocatedCString endpoint_;

    auto clone() const noexcept -> Pair* final;
    auto have_callback() const noexcept -> bool final;
    void process_incoming(const Lock& lock, Message&& message) noexcept final;

    void init() noexcept final;

    Pair() = delete;
    Pair(const Pair&) = delete;
    Pair(Pair&&) = delete;
    auto operator=(const Pair&) -> Pair& = delete;
    auto operator=(Pair&&) -> Pair& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
