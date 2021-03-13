// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "blockchain/p2p/Peer.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <iterator>
#include <string_view>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "util/Work.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::implementation::ZMQIncomingConnectionManager::"

namespace opentxs::blockchain::p2p::implementation
{
struct ZMQIncomingConnectionManager final : public Peer::ConnectionManager {
    const api::Core& api_;
    Peer& parent_;
    const Flag& running_;
    EndpointData endpoint_;
    const std::string zmq_;
    const std::size_t header_bytes_;
    std::promise<void> init_promise_;
    OTZMQListenCallback cb_;
    OTZMQDealerSocket dealer_;

    auto address() const noexcept -> std::string final { return "::1/128"; }
    auto endpoint_data() const noexcept -> EndpointData final
    {
        return endpoint_;
    }
    auto host() const noexcept -> std::string final { return endpoint_.first; }
    auto port() const noexcept -> std::uint16_t final
    {
        return endpoint_.second;
    }
    auto style() const noexcept -> p2p::Network final
    {
        return p2p::Network::zmq;
    }

    auto connect() noexcept -> void final {}
    auto init(const int id) noexcept -> bool final
    {
        auto future = init_promise_.get_future();
        auto zmq = dealer_->Start(zmq_);

        OT_ASSERT(zmq);

        auto message = MakeWork(api_, OT_ZMQ_REGISTER_SIGNAL);
        message->AddFrame(id);
        zmq = dealer_->Send(message);

        OT_ASSERT(zmq);

        const auto status = future.wait_for(std::chrono::seconds(10));

        return std::future_status::ready == status;
    }
    auto pipeline(zmq::Message& message) noexcept -> void
    {
        if (false == running_) { return; }

        const auto body = message.Body();

        OT_ASSERT(0 < body.size());

        switch (body.at(0).as<Peer::Task>()) {
            case Peer::Task::Register: {
                try {
                    init_promise_.set_value();
                } catch (...) {
                }

                parent_.on_connect();
            } break;
            case Peer::Task::Body: {
                OT_ASSERT(1 < body.size());

                parent_.on_pipeline(
                    Peer::Task::ReceiveMessage,
                    {body.at(1).Bytes(),
                     (2 < body.size()) ? body.at(2).Bytes() : ReadView{}});
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto shutdown_external() noexcept -> void final { dealer_->Close(); }
    auto stop_external() noexcept -> void final { dealer_->Close(); }
    auto stop_internal() noexcept -> void final {}
    auto transmit(
        const zmq::Frame& payload,
        std::shared_ptr<Peer::SendPromise> promise) noexcept -> void final
    {
        OT_ASSERT(header_bytes_ <= payload.size());

        const auto bytes = payload.Bytes();
        auto it = bytes.data();
        auto message = MakeWork(api_, OT_ZMQ_SEND_SIGNAL);
        message->AddFrame(it, header_bytes_);
        const auto body = bytes.size() - header_bytes_;

        if (0 < body) {
            std::advance(it, header_bytes_);
            message->AddFrame(it, body);
        }

        const auto sent = dealer_->Send(message);
        const auto ec = sent ? boost::system::error_code{}
                             : boost::system::error_code{
                                   boost::asio::error::host_unreachable};

        try {
            if (promise) { promise->set_value({ec, bytes.size()}); }
        } catch (...) {
        }
    }

    ZMQIncomingConnectionManager(
        const api::Core& api,
        Peer& parent,
        const Flag& running,
        const Peer::Address& address,
        const std::size_t headerSize) noexcept
        : api_(api)
        , parent_(parent)
        , running_(running)
        , endpoint_(address.Bytes()->str(), address.Port())
        , zmq_(endpoint_.first + ':' + std::to_string(endpoint_.second))
        , header_bytes_(headerSize)
        , init_promise_()
        , cb_(zmq::ListenCallback::Factory(
              [&](auto& in) { this->pipeline(in); }))
        , dealer_(api.ZeroMQ().DealerSocket(
              cb_,
              zmq::socket::Socket::Direction::Connect))
    {
    }

    ~ZMQIncomingConnectionManager() final
    {
        stop_internal();
        stop_external();
        shutdown_external();
    }
};

auto Peer::ConnectionManager::ZMQIncoming(
    const api::Core& api,
    Peer& parent,
    const Flag& running,
    const Peer::Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<ZMQIncomingConnectionManager>(
        api, parent, running, address, headerSize);
}
}  // namespace opentxs::blockchain::p2p::implementation
