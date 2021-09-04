// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <utility>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Sender.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

// #define OT_METHOD
// "opentxs::blockchain::node::implementation::PeerManager::Jobs::"

namespace opentxs::blockchain::node::implementation
{
PeerManager::Jobs::Jobs(const api::Core& api) noexcept
    : zmq_(api.Network().ZeroMQ())
    , getheaders_(zmq_.PushSocket(zmq::socket::Socket::Direction::Bind))
    , getcfheaders_(zmq_.PublishSocket())
    , getcfilters_(zmq_.PublishSocket())
    , getblocks_(zmq_.PublishSocket())
    , heartbeat_(zmq_.PublishSocket())
    , getblock_(zmq_.PushSocket(zmq::socket::Socket::Direction::Bind))
    , broadcast_transaction_(
          zmq_.PushSocket(zmq::socket::Socket::Direction::Bind))
    , broadcast_block_(zmq_.PublishSocket())
    , endpoint_map_([&] {
        auto map = EndpointMap{};
        listen(map, Task::Getheaders, getheaders_);
        listen(map, Task::JobAvailableCfheaders, getcfheaders_);
        listen(map, Task::JobAvailableCfilters, getcfilters_);
        listen(map, Task::JobAvailableBlock, getblocks_);
        listen(map, Task::Heartbeat, heartbeat_);
        listen(map, Task::Getblock, getblock_);
        listen(map, Task::BroadcastTransaction, broadcast_transaction_);
        listen(map, Task::BroadcastBlock, broadcast_block_);

        return map;
    }())
    , socket_map_({
          {Task::Getheaders, &getheaders_.get()},
          {Task::JobAvailableCfheaders, &getcfheaders_.get()},
          {Task::JobAvailableBlock, &getcfilters_.get()},
          {Task::JobAvailableCfilters, &getblocks_.get()},
          {Task::Heartbeat, &heartbeat_.get()},
          {Task::Getblock, &getblock_.get()},
          {Task::BroadcastTransaction, &broadcast_transaction_.get()},
          {Task::BroadcastBlock, &broadcast_block_.get()},
      })
{
}

auto PeerManager::Jobs::Dispatch(const Task type) noexcept -> void
{
    Dispatch(Work(type));
}

auto PeerManager::Jobs::Dispatch(zmq::Message& work) noexcept -> void
{
    const auto body = work.Body();

    OT_ASSERT(0 < body.size());

    const auto task = [&] {
        try {

            return body.at(0).as<Task>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    socket_map_.at(task)->Send(work);
}

auto PeerManager::Jobs::Endpoint(const Task type) const noexcept -> std::string
{
    try {

        return endpoint_map_.at(type);
    } catch (...) {

        return {};
    }
}

auto PeerManager::Jobs::listen(
    EndpointMap& map,
    const Task type,
    const zmq::socket::Sender& socket) noexcept -> void
{
    auto [it, added] = map.emplace(
        type,
        std::string{"inproc://opentxs//blockchain/peer_tasks/"} +
            Identifier::Random()->str());

    OT_ASSERT(added);

    const auto listen = socket.Start(it->second);

    OT_ASSERT(listen);
}

auto PeerManager::Jobs::Shutdown() noexcept -> void
{
    for (auto [type, socket] : socket_map_) { socket->Close(); }
}

auto PeerManager::Jobs::Work(const Task task, std::promise<void>* promise)
    const noexcept -> OTZMQMessage
{
    if (nullptr != promise) {
        return zmq_.TaggedReply(
            ReadView{reinterpret_cast<char*>(promise), sizeof(promise)}, task);
    } else {
        return zmq_.TaggedMessage(task);
    }
}
}  // namespace opentxs::blockchain::node::implementation
