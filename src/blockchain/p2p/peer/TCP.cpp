// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <cstddef>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::p2p::implementation::TCPConnectionManager::"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace opentxs::blockchain::p2p::implementation
{
struct TCPConnectionManager : virtual public Peer::ConnectionManager {
    const api::Core& api_;
    Peer& parent_;
    const Flag& running_;
    const network::asio::Endpoint endpoint_;
    const Space connection_id_;
    const std::size_t header_bytes_;
    std::promise<void> connection_id_promise_;
    network::asio::Socket socket_;
    OTData header_;
    OTZMQListenCallback cb_;
    OTZMQDealerSocket dealer_;

    auto address() const noexcept -> std::string final
    {
        return endpoint_.GetMapped();
    }
    auto endpoint_data() const noexcept -> EndpointData final
    {
        return {address(), port()};
    }
    auto host() const noexcept -> std::string final
    {
        return endpoint_.GetAddress();
    }
    auto port() const noexcept -> std::uint16_t final
    {
        return endpoint_.GetPort();
    }
    auto style() const noexcept -> p2p::Network final
    {
        return p2p::Network::ipv6;
    }

    auto connect() noexcept -> void override
    {
        if (0 < connection_id_.size()) {
            LogVerbose(OT_METHOD)(__func__)(": Connecting to ")(endpoint_.str())
                .Flush();
            socket_.Connect(reader(connection_id_));
        }
    }
    auto init(const int id) noexcept -> bool final
    {
        auto future = connection_id_promise_.get_future();
        auto zmq = dealer_->Start(api_.Network().Asio().NotificationEndpoint());

        OT_ASSERT(zmq);

        auto message = MakeWork(api_, WorkType::AsioRegister);
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

        const auto task = [&] {
            try {

                return body.at(0).as<Peer::Task>();
            } catch (...) {

                OT_FAIL;
            }
        }();

        switch (task) {
            case Peer::Task::Register: {
                OT_ASSERT(1 < body.size());

                const auto& id = body.at(1);

                OT_ASSERT(0 < id.size());

                const auto start = static_cast<const std::byte*>(id.data());
                const_cast<Space&>(connection_id_)
                    .assign(start, start + id.size());

                OT_ASSERT(0 < connection_id_.size());

                try {
                    connection_id_promise_.set_value();
                } catch (...) {
                }
            } break;
            case Peer::Task::Connect: {
                LogVerbose(OT_METHOD)(__func__)(": Connect to ")(
                    endpoint_.str())(" successful")
                    .Flush();
                parent_.on_connect();
                run();
            } break;
            case Peer::Task::Disconnect: {
                parent_.on_pipeline(Peer::Task::Disconnect, {});
            } break;
            case Peer::Task::Header: {
                OT_ASSERT(1 < body.size());

                const auto& messageHeader = body.at(1);
                const auto size = parent_.get_body_size(messageHeader);

                if (0 < size) {
                    header_->Assign(messageHeader.Bytes());
                    receive(static_cast<OTZMQWorkType>(Peer::Task::Body), size);
                    parent_.on_pipeline(Peer::Task::Header, {});
                } else {
                    parent_.on_pipeline(
                        Peer::Task::ReceiveMessage,
                        {messageHeader.Bytes(), {}});
                    run();
                }
            } break;
            case Peer::Task::Body: {
                OT_ASSERT(1 < body.size());

                parent_.on_pipeline(
                    Peer::Task::ReceiveMessage,
                    {header_->Bytes(), body.at(1).Bytes()});
                run();
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto receive(const OTZMQWorkType type, const std::size_t bytes) noexcept
        -> void
    {
        socket_.Receive(reader(connection_id_), type, bytes);
    }
    auto run() noexcept -> void
    {
        if (running_) {
            receive(
                static_cast<OTZMQWorkType>(Peer::Task::Header), header_bytes_);
        }
    }
    auto shutdown_external() noexcept -> void final { socket_.Close(); }
    auto stop_external() noexcept -> void final { socket_.Close(); }
    auto stop_internal() noexcept -> void final { dealer_->Close(); }
    auto transmit(
        const zmq::Frame& data,
        std::unique_ptr<Peer::SendPromise> promise) noexcept -> void final
    {
        socket_.Transmit(data.Bytes(), std::move(promise));
    }

    TCPConnectionManager(
        const api::Core& api,
        Peer& parent,
        const Flag& running,
        const Peer::Address& address,
        const std::size_t headerSize) noexcept
        : api_(api)
        , parent_(parent)
        , running_(running)
        , endpoint_(make_endpoint(address))
        , connection_id_()
        , header_bytes_(headerSize)
        , connection_id_promise_()
        , socket_(api_.Network().Asio().MakeSocket(endpoint_))
        , header_([&] {
            auto out = api_.Factory().Data();
            out->SetSize(headerSize);

            return out;
        }())
        , cb_(zmq::ListenCallback::Factory(
              [&](auto& in) { this->pipeline(in); }))
        , dealer_(api_.Network().ZeroMQ().DealerSocket(
              cb_,
              zmq::socket::Socket::Direction::Connect))
    {
    }

    ~TCPConnectionManager() override
    {
        stop_internal();
        socket_.Close();
    }

protected:
    static auto make_endpoint(const Peer::Address& address) noexcept
        -> network::asio::Endpoint
    {
        using Type = network::asio::Endpoint::Type;

        switch (address.Type()) {
            case p2p::Network::ipv6: {

                return {Type::ipv6, address.Bytes()->Bytes(), address.Port()};
            }
            case p2p::Network::ipv4: {

                return {Type::ipv4, address.Bytes()->Bytes(), address.Port()};
            }
            default: {

                return {};
            }
        }
    }

    TCPConnectionManager(
        const api::Core& api,
        const Flag& running,
        const std::size_t headerSize,
        Peer& parent,
        network::asio::Endpoint&& endpoint,
        network::asio::Socket&& socket) noexcept
        : api_(api)
        , parent_(parent)
        , running_(running)
        , endpoint_(std::move(endpoint))
        , connection_id_()
        , header_bytes_(headerSize)
        , connection_id_promise_()
        , socket_(std::move(socket))
        , header_([&] {
            auto out = api_.Factory().Data();
            out->SetSize(headerSize);

            return out;
        }())
        , cb_(zmq::ListenCallback::Factory(
              [&](auto& in) { this->pipeline(in); }))
        , dealer_(api_.Network().ZeroMQ().DealerSocket(
              cb_,
              zmq::socket::Socket::Direction::Connect))
    {
    }
};

struct TCPIncomingConnectionManager final : public TCPConnectionManager {
    auto connect() noexcept -> void final {}

    TCPIncomingConnectionManager(
        const api::Core& api,
        Peer& parent,
        const Flag& running,
        const Peer::Address& address,
        const std::size_t headerSize,
        network::asio::Socket&& socket) noexcept
        : TCPConnectionManager(
              api,
              running,
              headerSize,
              parent,
              make_endpoint(address),
              std::move(socket))
    {
    }

    ~TCPIncomingConnectionManager() final = default;
};

auto Peer::ConnectionManager::TCP(
    const api::Core& api,
    Peer& parent,
    const Flag& running,
    const Peer::Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPConnectionManager>(
        api, parent, running, address, headerSize);
}

auto Peer::ConnectionManager::TCPIncoming(
    const api::Core& api,
    Peer& parent,
    const Flag& running,
    const Peer::Address& address,
    const std::size_t headerSize,
    opentxs::network::asio::Socket&& socket) noexcept
    -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPIncomingConnectionManager>(
        api, parent, running, address, headerSize, std::move(socket));
}
}  // namespace opentxs::blockchain::p2p::implementation
