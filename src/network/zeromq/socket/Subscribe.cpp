// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "network/zeromq/socket/Subscribe.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>

#include "internal/network/zeromq/socket/Socket.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>;

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::socket::implementation::Subscribe::"

namespace opentxs::factory
{
auto SubscribeSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Subscribe>
{
    using ReturnType = network::zeromq::socket::implementation::Subscribe;

    return std::make_unique<ReturnType>(context, callback);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Subscribe::Subscribe(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback) noexcept
    : Receiver(context, SocketType::Subscribe, Socket::Direction::Connect, true)
    , Client(this->get())
    , callback_(callback)
{
    init();
}

auto Subscribe::clone() const noexcept -> Subscribe*
{
    return new Subscribe(context_, callback_);
}

auto Subscribe::have_callback() const noexcept -> bool { return true; }

void Subscribe::init() noexcept
{
    // subscribe to all messages until filtering is implemented
    const auto set = zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);
    Receiver::init();

    OT_ASSERT(0 == set);
}

void Subscribe::process_incoming(const Lock& lock, Message& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    try {
        callback_.Process(message);
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": Callback exception: ")(e.what())
            .Flush();

        for (const auto& endpoint : endpoints_) {
            LogOutput(OT_METHOD)(__func__)(": connected endpoint: ")(endpoint)
                .Flush();
        }
    } catch (...) {
        LogOutput(OT_METHOD)(__func__)(": Callback exception").Flush();
    }
}

auto Subscribe::SetSocksProxy(const std::string& proxy) const noexcept -> bool
{
    return set_socks_proxy(proxy);
}

Subscribe::~Subscribe() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
