// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "network/zeromq/Proxy.hpp"  // IWYU pragma: associated

#include <zmq.h>

#include "internal/network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Proxy>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::Proxy::"

namespace opentxs::network::zeromq
{
auto Proxy::Factory(
    const zeromq::Context& context,
    socket::Socket& frontend,
    socket::Socket& backend) -> OTZMQProxy
{
    return OTZMQProxy(new implementation::Proxy(context, frontend, backend));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Proxy::Proxy(
    const zeromq::Context& context,
    zeromq::socket::Socket& frontend,
    zeromq::socket::Socket& backend)
    : context_(context)
    , frontend_(frontend)
    , backend_(backend)
    , null_callback_(opentxs::network::zeromq::ListenCallback::Factory(
          [](const zeromq::Message&) -> void {}))
    , control_listener_(factory::PairSocket(context, null_callback_, false))
    , control_sender_(
          factory::PairSocket(null_callback_, control_listener_, false))
    , thread_(nullptr)
{
    thread_.reset(new std::thread(&Proxy::proxy, this));

    OT_ASSERT(thread_)
}

auto Proxy::clone() const -> Proxy*
{
    return new Proxy(context_, frontend_, backend_);
}

void Proxy::proxy() const
{
    zmq_proxy_steerable(frontend_, backend_, nullptr, nullptr);
}

Proxy::~Proxy()
{
    control_sender_->Send("TERMINATE");

    if (thread_ && thread_->joinable()) { thread_->join(); }
}
}  // namespace opentxs::network::zeromq::implementation
