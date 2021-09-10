// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

#include <iterator>
#include <string_view>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::p2p::implementation::ZMQConnectionManager::"

namespace opentxs::blockchain::p2p::implementation
{
struct ZMQConnectionManager : virtual public Peer::ConnectionManager {
    const api::Core& api_;
    Peer& parent_;
    const Flag& running_;
    EndpointData endpoint_;
    const std::string zmq_;
    const std::size_t header_bytes_;
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

    auto connect() noexcept -> void override
    {
        LogVerbose(OT_METHOD)(__func__)(": Connecting to ")(zmq_).Flush();
        dealer_->Start(zmq_);
        parent_.on_connect();
    }
    auto init(const int id) noexcept -> bool override { return true; }
    virtual auto pipeline(zmq::Message& message) noexcept -> void
    {
        if (false == running_) { return; }

        const auto body = message.Body();

        if (1 > body.size()) { return; }

        parent_.on_pipeline(
            Peer::Task::ReceiveMessage,
            {body.at(0).Bytes(),
             (1 < body.size()) ? body.at(1).Bytes() : ReadView{}});
    }
    auto shutdown_external() noexcept -> void final { dealer_->Close(); }
    auto stop_external() noexcept -> void final { dealer_->Close(); }
    auto stop_internal() noexcept -> void final {}
    auto transmit(
        const zmq::Frame& payload,
        std::unique_ptr<Peer::SendPromise> promise) noexcept -> void final
    {
        OT_ASSERT(header_bytes_ <= payload.size());

        const auto sent = dealer_->Send(make_outgoing_message(payload));

        try {
            if (promise) { promise->set_value(sent); }
        } catch (...) {
        }
    }

    ZMQConnectionManager(
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
        , cb_(zmq::ListenCallback::Factory(
              [&](auto& in) { this->pipeline(in); }))
        , dealer_(api.Network().ZeroMQ().DealerSocket(
              cb_,
              zmq::socket::Socket::Direction::Connect))
    {
    }

    ~ZMQConnectionManager() override
    {
        stop_internal();
        stop_external();
        shutdown_external();
    }

private:
    virtual auto make_outgoing_message(const zmq::Frame& payload) const noexcept
        -> OTZMQMessage
    {
        const auto bytes = payload.Bytes();
        auto it = bytes.data();
        auto message = api_.Network().ZeroMQ().Message();
        message->PrependEmptyFrame();
        message->AddFrame(it, header_bytes_);
        const auto body = bytes.size() - header_bytes_;

        if (0 < body) {
            std::advance(it, header_bytes_);
            message->AddFrame(it, body);
        }

        return message;
    }
};

struct ZMQIncomingConnectionManager final : public ZMQConnectionManager {
    std::promise<void> init_promise_;

    auto connect() noexcept -> void final {}
    auto init(const int id) noexcept -> bool final
    {
        auto out = ZMQConnectionManager::init(id);
        auto future = init_promise_.get_future();
        auto zmq = dealer_->Start(zmq_);

        OT_ASSERT(zmq);

        auto message = MakeWork(api_, WorkType::AsioRegister);
        message->AddFrame(id);
        zmq = dealer_->Send(message);

        OT_ASSERT(zmq);

        const auto status = future.wait_for(std::chrono::seconds(10));

        return (std::future_status::ready == status) && out;
    }
    auto pipeline(zmq::Message& message) noexcept -> void final
    {
        if (false == running_) { return; }

        const auto body = message.Body();

        OT_ASSERT(0 < body.size());

        const auto task = [&] {
            try {

                return body.at(0).as<Peer::Task>();
            } catch (...) {

                OT_FAIL;
            }
        }();

        switch (task) {
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

    ZMQIncomingConnectionManager(
        const api::Core& api,
        Peer& parent,
        const Flag& running,
        const Peer::Address& address,
        const std::size_t headerSize) noexcept
        : ZMQConnectionManager(api, parent, running, address, headerSize)
        , init_promise_()
    {
    }

    ~ZMQIncomingConnectionManager() final = default;

private:
    auto make_outgoing_message(const zmq::Frame& payload) const noexcept
        -> OTZMQMessage final
    {
        const auto bytes = payload.Bytes();
        auto it = bytes.data();
        auto message = MakeWork(api_, OT_ZMQ_SEND_SIGNAL);
        message->AddFrame(it, header_bytes_);
        const auto body = bytes.size() - header_bytes_;

        if (0 < body) {
            std::advance(it, header_bytes_);
            message->AddFrame(it, body);
        }

        return message;
    }
};

auto Peer::ConnectionManager::ZMQ(
    const api::Core& api,
    Peer& parent,
    const Flag& running,
    const Peer::Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<ZMQConnectionManager>(
        api, parent, running, address, headerSize);
}

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
