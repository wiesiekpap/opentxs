// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "blockchain/p2p/Peer.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstring>
#include <stdexcept>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

#define OT_METHOD                                                              \
    "opentxs::blockchain::p2p::implementation::TCPConnectionManager::"

namespace opentxs::blockchain::p2p::implementation
{
struct TCPConnectionManager final : public Peer::ConnectionManager {
    const api::Core& api_;
    Peer& parent_;
    const Flag& running_;
    const tcp::endpoint endpoint_;
    const blockchain::client::internal::IO& context_;
    const Space connection_id_;
    const std::size_t header_bytes_;
    std::promise<void> connection_id_promise_;
    tcp::socket socket_;
    OTData header_;
    OTZMQListenCallback cb_;
    OTZMQDealerSocket dealer_;

    static auto make_endpoint(
        const Network type,
        const Data& raw,
        const std::uint16_t port) noexcept -> tcp::endpoint
    {
        auto output = ip::address_v6{};

        switch (type) {
            case p2p::Network::ipv6: {
                auto bytes = ip::address_v6::bytes_type{};

                OT_ASSERT(bytes.size() == raw.size());

                std::memcpy(&bytes, raw.data(), bytes.size());
                output = ip::make_address_v6(bytes);
            } break;
            case p2p::Network::ipv4: {
                auto output4 = ip::address_v4{};
                auto bytes = ip::address_v4::bytes_type{};

                OT_ASSERT(bytes.size() == raw.size());

                std::memcpy(&bytes, raw.data(), bytes.size());
                output4 = ip::make_address_v4(bytes);
                output = ip::make_address_v6(
                    std::string("::ffff:") + output4.to_string());
            } break;
            default: {
                OT_FAIL;
            }
        }

        return ip::tcp::endpoint{output, port};
    }
    static auto make_buffer(const std::size_t size) noexcept -> OTData
    {
        auto output = Data::Factory();
        output->SetSize(size);

        return output;
    }

    auto address() const noexcept -> std::string final
    {
        return endpoint_.address().to_string();
    }
    auto endpoint_data() const noexcept -> EndpointData final
    {
        try {
            const auto local = socket_.local_endpoint();

            return {local.address().to_v6().to_string(), local.port()};
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
            OT_FAIL;
        }
    }
    auto host() const noexcept -> std::string final
    {
        return endpoint_.address().to_string();
    }
    auto port() const noexcept -> std::uint16_t final
    {
        return endpoint_.port();
    }
    auto style() const noexcept -> p2p::Network final
    {
        return p2p::Network::ipv6;
    }

    auto connect() noexcept -> void final
    {
        if (0 < connection_id_.size()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Connecting to ")(
                endpoint_.address().to_string())(":")(endpoint_.port())
                .Flush();
            context_.Connect(connection_id_, endpoint_, socket_);
        }
    }
    auto init(const int id) noexcept -> bool final
    {
        auto future = connection_id_promise_.get_future();
        auto zmq =
            dealer_->Start(api_.Endpoints().InternalBlockchainAsioContext());

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
                LogVerbose(OT_METHOD)(__FUNCTION__)(": Connect to ")(
                    endpoint_.address().to_string())(":")(endpoint_.port())(
                    " successful")
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
        context_.Receive(connection_id_, type, bytes, socket_);
    }
    auto run() noexcept -> void
    {
        if (running_) {
            receive(
                static_cast<OTZMQWorkType>(Peer::Task::Header), header_bytes_);
        }
    }
    auto shutdown_external() noexcept -> void final
    {
        try {
            socket_.close();
        } catch (...) {
        }
    }
    auto stop_external() noexcept -> void final
    {
        try {
            socket_.shutdown(tcp::socket::shutdown_both);
        } catch (...) {
        }
    }
    auto stop_internal() noexcept -> void final { dealer_->Close(); }
    auto transmit(
        const zmq::Frame& data,
        std::shared_ptr<Peer::SendPromise> promise) noexcept -> void final
    {
        auto it = static_cast<const std::byte*>(data.data());
        auto bytes = std::make_unique<Space>(it, it + data.size());
        auto work = [=, b{bytes.release()}]() -> void {
            auto cb = [=](auto& error, auto bytes) -> void {
                auto cleanup = std::unique_ptr<Space>{b};

                try {
                    if (promise) { promise->set_value({error, bytes}); }
                } catch (...) {
                }
            };
            asio::async_write(socket_, asio::buffer(b->data(), b->size()), cb);
        };

        auto& asio = context_.operator boost::asio::io_context&();
        asio.post(work);
    }

    TCPConnectionManager(
        const api::Core& api,
        Peer& parent,
        const Flag& running,
        const Peer::Address& address,
        const std::size_t headerSize,
        const blockchain::client::internal::IO& context) noexcept
        : api_(api)
        , parent_(parent)
        , running_(running)
        , endpoint_(
              make_endpoint(address.Type(), address.Bytes(), address.Port()))
        , context_(context)
        , connection_id_()
        , header_bytes_(headerSize)
        , connection_id_promise_()
        , socket_(context_.operator boost::asio::io_context&())
        , header_(make_buffer(headerSize))
        , cb_(zmq::ListenCallback::Factory(
              [&](auto& in) { this->pipeline(in); }))
        , dealer_(api.ZeroMQ().DealerSocket(
              cb_,
              zmq::socket::Socket::Direction::Connect))
    {
    }

    ~TCPConnectionManager() final
    {
        stop_internal();
        stop_external();
        shutdown_external();
    }
};

auto Peer::ConnectionManager::TCP(
    const api::Core& api,
    Peer& parent,
    const Flag& running,
    const Peer::Address& address,
    const std::size_t headerSize,
    const blockchain::client::internal::IO& context) noexcept
    -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPConnectionManager>(
        api, parent, running, address, headerSize, context);
}
}  // namespace opentxs::blockchain::p2p::implementation
