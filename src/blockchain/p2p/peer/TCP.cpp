// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "blockchain/p2p/peer/ConnectionManager.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <chrono>
#include <cstddef>
#include <string_view>

#include "blockchain/p2p/peer/Address.hpp"
#include "blockchain/p2p/peer/Peer.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"
#include "util/threadutil.hpp"

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

namespace opentxs::blockchain::p2p::peer
{
using namespace std::literals;

struct TCPConnectionManager : virtual public ConnectionManager {
    const api::Session& api_;
    const int id_;
    p2p::implementation::Peer& parent_;
    network::zeromq::Pipeline& pipeline_;
    const std::atomic<bool>& running_;
    const network::asio::Endpoint endpoint_;
    const Space connection_id_;
    const std::size_t header_bytes_;
    std::promise<void> connection_id_promise_;
    std::shared_future<void> connection_id_future_;
    network::asio::Socket socket_;
    OTData header_;

    auto address() const noexcept -> UnallocatedCString final
    {
        return endpoint_.GetMapped();
    }
    auto endpoint_data() const noexcept -> EndpointData final
    {
        return {address(), port()};
    }
    auto filter(const Task) const noexcept -> bool final { return true; }
    auto host() const noexcept -> UnallocatedCString final
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
        OT_ASSERT(0 < connection_id_.size());

        LogVerbose()(OT_PRETTY_CLASS())("Connecting to ")(endpoint_.str())
            .Flush();
        socket_.Connect(reader(connection_id_));
    }
    auto init() noexcept -> void final
    {
        pipeline_.ConnectDealer(
            api_.Network().Asio().NotificationEndpoint(), [id = id_](auto) {
                const network::zeromq::SocketID header = id;
                auto out = zmq::Message{};
                out.AddFrame(header);
                out.StartBody();
                out.AddFrame(Task::Init);

                return out;
            });
    }
    auto is_initialized() const noexcept -> bool final
    {
        static constexpr auto zero = 0ns;
        static constexpr auto ready = std::future_status::ready;

        return (ready == connection_id_future_.wait_for(zero));
    }
    auto on_body(zmq::Message&& message) noexcept -> void final
    {
        MessageMarker mm(message);
        if (mm) {
            std::cerr << "TCPConnectionManager::on_body received from "
                      << to_string(mm) << "\n";
        }
        auto body = message.Body();

        OT_ASSERT(1 < body.size());

        parent_.process_message([&] {
            auto out = zmq::Message{};
            out.StartBody();
            out.AddFrame(Task::P2P);
            out.AddFrame(header_);
            out.AddFrame(std::move(body.at(1)));

            MessageMarker m("T..body\0"sv);
            m.mark(out);

            return out;
        }());
        run();
    }
    auto on_connect(zmq::Message&&) noexcept -> void final
    {
        LogVerbose()(OT_PRETTY_CLASS())("Connect to ")(endpoint_.str())(
            " successful")
            .Flush();
        parent_.on_connect();
        run();
    }
    auto on_header(zmq::Message&& message) noexcept -> void final
    {
        MessageMarker mm(message);
        if (mm) {
            std::cerr << "TCPConnectionManager::on_header received from "
                      << to_string(mm) << "\n";
        }
        auto body = message.Body();

        OT_ASSERT(1 < body.size());

        auto& header = body.at(1);
        const auto size = parent_.get_body_size(header);

        if (0 < size) {
            header_->Assign(header.Bytes());
            receive(static_cast<OTZMQWorkType>(Task::Body), size);
        } else {
            parent_.process_message([&] {
                auto out = zmq::Message{};
                out.StartBody();
                out.AddFrame(Task::P2P);
                out.AddFrame(std::move(header));
                out.AddFrame();

                MessageMarker m("QQQ TCPConnectionManager::on_header"sv);
                m.mark(out);

                return out;
            }());
            run();
        }
    }
    auto on_init(zmq::Message&&) noexcept -> void final
    {
        pipeline_.Send([&] {
            auto out = MakeWork(WorkType::AsioRegister);
            out.AddFrame(id_);

            return out;
        }());
    }
    auto on_register(zmq::Message&& message) noexcept -> void final
    {
        const auto body = message.Body();

        OT_ASSERT(1 < body.size());

        const auto& id = body.at(1);

        OT_ASSERT(0 < id.size());

        const auto start = static_cast<const std::byte*>(id.data());
        const_cast<Space&>(connection_id_).assign(start, start + id.size());

        OT_ASSERT(0 < connection_id_.size());

        try {
            connection_id_promise_.set_value();
        } catch (...) {
        }

        parent_.on_init();
    }
    auto receive(const OTZMQWorkType type, const std::size_t bytes) noexcept
        -> void
    {
        socket_.Receive(reader(connection_id_), type, bytes);
    }
    auto run() noexcept -> void
    {
        if (running_) {
            receive(static_cast<OTZMQWorkType>(Task::Header), header_bytes_);
        }
    }
    auto shutdown_external() noexcept -> void final { socket_.Close(); }
    auto stop_external() noexcept -> void final { socket_.Close(); }
    auto transmit(
        zmq::Frame&& header,
        zmq::Frame&& payload,
        std::unique_ptr<SendPromise> promise) noexcept -> void final
    {
        header += payload;
        socket_.Transmit(header.Bytes(), std::move(promise));
    }

    TCPConnectionManager(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize) noexcept
        : api_(api)
        , id_(id)
        , parent_(parent)
        , pipeline_(pipeline)
        , running_(running)
        , endpoint_(make_endpoint(address))
        , connection_id_()
        , header_bytes_(headerSize)
        , connection_id_promise_()
        , connection_id_future_(connection_id_promise_.get_future())
        , socket_(api_.Network().Asio().MakeSocket(endpoint_))
        , header_([&] {
            auto out = api_.Factory().Data();
            out->SetSize(headerSize);

            return out;
        }())
    {
    }

    ~TCPConnectionManager() override { socket_.Close(); }

protected:
    static auto make_endpoint(const Address& address) noexcept
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
        const api::Session& api,
        const int id,
        const std::atomic<bool>& running,
        const std::size_t headerSize,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        network::asio::Endpoint&& endpoint,
        network::asio::Socket&& socket) noexcept
        : api_(api)
        , id_(id)
        , parent_(parent)
        , pipeline_(pipeline)
        , running_(running)
        , endpoint_(std::move(endpoint))
        , connection_id_()
        , header_bytes_(headerSize)
        , connection_id_promise_()
        , connection_id_future_(connection_id_promise_.get_future())
        , socket_(std::move(socket))
        , header_([&] {
            auto out = api_.Factory().Data();
            out->SetSize(headerSize);

            return out;
        }())
    {
    }
};

struct TCPIncomingConnectionManager final : public TCPConnectionManager {
    auto connect() noexcept -> void final {}

    TCPIncomingConnectionManager(
        const api::Session& api,
        const int id,
        p2p::implementation::Peer& parent,
        network::zeromq::Pipeline& pipeline,
        const std::atomic<bool>& running,
        const Address& address,
        const std::size_t headerSize,
        network::asio::Socket&& socket) noexcept
        : TCPConnectionManager(
              api,
              id,
              running,
              headerSize,
              parent,
              pipeline,
              make_endpoint(address),
              std::move(socket))
    {
    }

    ~TCPIncomingConnectionManager() final = default;
};

auto ConnectionManager::TCP(
    const api::Session& api,
    const int id,
    p2p::implementation::Peer& parent,
    network::zeromq::Pipeline& pipeline,
    const std::atomic<bool>& running,
    const Address& address,
    const std::size_t headerSize) noexcept -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPConnectionManager>(
        api, id, parent, pipeline, running, address, headerSize);
}

auto ConnectionManager::TCPIncoming(
    const api::Session& api,
    const int id,
    p2p::implementation::Peer& parent,
    network::zeromq::Pipeline& pipeline,
    const std::atomic<bool>& running,
    const Address& address,
    const std::size_t headerSize,
    opentxs::network::asio::Socket&& socket) noexcept
    -> std::unique_ptr<ConnectionManager>
{
    return std::make_unique<TCPIncomingConnectionManager>(
        api,
        id,
        parent,
        pipeline,
        running,
        address,
        headerSize,
        std::move(socket));
}
}  // namespace opentxs::blockchain::p2p::peer
