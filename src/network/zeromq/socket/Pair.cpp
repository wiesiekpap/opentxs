// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "network/zeromq/socket/Pair.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/network/zeromq/socket/Socket.hpp"
#include "network/zeromq/socket/Bidirectional.tpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Sender.tpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Pair>;

// #define OT_METHOD
// "opentxs::network::zeromq::socket::implementation::Pair::"

namespace opentxs::factory
{
auto PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const bool startThread) -> std::unique_ptr<network::zeromq::socket::Pair>
{
    using ReturnType = network::zeromq::socket::implementation::Pair;

    return std::make_unique<ReturnType>(context, callback, startThread);
}

auto PairSocket(
    const network::zeromq::ListenCallback& callback,
    const network::zeromq::socket::Pair& peer,
    const bool startThread) -> std::unique_ptr<network::zeromq::socket::Pair>
{
    using ReturnType = network::zeromq::socket::implementation::Pair;

    return std::make_unique<ReturnType>(callback, peer, startThread);
}

auto PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const std::string& endpoint)
    -> std::unique_ptr<network::zeromq::socket::Pair>
{
    using ReturnType = network::zeromq::socket::implementation::Pair;

    return std::make_unique<ReturnType>(context, callback, endpoint);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Pair::Pair(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint,
    const Socket::Direction direction,
    const bool startThread) noexcept
    : Receiver(context, SocketType::Pair, direction, startThread)
    , Bidirectional(context, true)
    , callback_(callback)
    , endpoint_(endpoint)
{
    init();
}

Pair::Pair(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const bool startThread) noexcept
    : Pair(
          context,
          callback,
          Socket::random_inproc_endpoint(),
          Socket::Direction::Bind,
          startThread)
{
}

Pair::Pair(
    const zeromq::ListenCallback& callback,
    const zeromq::socket::Pair& peer,
    const bool startThread) noexcept
    : Pair(
          peer.Context(),
          callback,
          peer.Endpoint(),
          Socket::Direction::Connect,
          startThread)
{
}

Pair::Pair(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint) noexcept
    : Pair(context, callback, endpoint, Socket::Direction::Connect, true)
{
}

auto Pair::clone() const noexcept -> Pair*
{
    return new Pair(context_, callback_, endpoint_, direction_, false);
}

auto Pair::Endpoint() const noexcept -> const std::string& { return endpoint_; }

auto Pair::have_callback() const noexcept -> bool { return true; }

void Pair::init() noexcept
{
    Bidirectional::init();

    OT_ASSERT(false == endpoint_.empty())

    const auto init = Bidirectional::Start(endpoint_);

    OT_ASSERT(init)
}

void Pair::process_incoming(const Lock& lock, Message& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

Pair::~Pair() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
