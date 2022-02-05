// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <cxxabi.h>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/network/blockchain/syncclientrouter/SyncClientRouter.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>
#include <zmq.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iterator>
#include <memory>
#include <random>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

#include "Proto.tpp"
#include "core/Worker.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/Query.hpp"
#include "opentxs/network/p2p/Request.hpp"
#include "opentxs/network/p2p/State.hpp"
#include "opentxs/network/p2p/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/BlockchainP2PChainState.pb.h"

namespace bc = opentxs::blockchain;

namespace opentxs::api::network::blockchain
{
SyncClientRouter::Imp::Imp(
    const api::Session& api,
    opentxs::network::zeromq::internal::Batch& batch) noexcept
    : api_(api)
    , endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
    , monitor_endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
    , loopback_endpoint_(opentxs::network::zeromq::MakeArbitraryInproc())
    , batch_([&]() -> auto& {
        auto& out = batch;
        out.listen_callbacks_.reserve(3);
        out.listen_callbacks_.emplace_back(Callback::Factory(
            [this](auto&& in) { process_external(std::move(in)); }));
        out.listen_callbacks_.emplace_back(Callback::Factory(
            [this](auto&& in) { process_internal(std::move(in)); }));
        out.listen_callbacks_.emplace_back(Callback::Factory(
            [this](auto&& in) { process_monitor(std::move(in)); }));

        return out;
    }())
    , external_cb_(batch_.listen_callbacks_.at(0))
    , internal_cb_(batch_.listen_callbacks_.at(1))
    , monitor_cb_(batch_.listen_callbacks_.at(2))
    , external_router_([&]() -> auto& {
        auto& out = batch_.sockets_.at(0);
        auto rc = out.SetRouterHandover(true);

        OT_ASSERT(rc);

        rc = out.SetMonitor(
            monitor_endpoint_.c_str(), ZMQ_EVENT_HANDSHAKE_SUCCEEDED);

        OT_ASSERT(rc);

        return out;
    }())
    , monitor_([&]() -> auto& {
        auto& out = batch_.sockets_.at(1);
        auto rc = out.Connect(monitor_endpoint_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , external_sub_([&]() -> auto& {
        auto& out = batch_.sockets_.at(2);
        auto rc = out.ClearSubscriptions();

        OT_ASSERT(rc);

        return out;
    }())
    , internal_router_([&]() -> auto& {
        auto& out = batch_.sockets_.at(3);
        auto rc = out.Bind(endpoint_.c_str());

        OT_ASSERT(rc);

        LogTrace()(OT_PRETTY_CLASS())("internal router bound to ")(endpoint_)
            .Flush();

        return out;
    }())
    , internal_sub_([&]() -> auto& {
        auto& out = batch_.sockets_.at(4);
        auto rc = out.ClearSubscriptions();

        OT_ASSERT(rc);

        rc =
            out.Connect(api_.Endpoints().BlockchainSyncServerUpdated().c_str());

        OT_ASSERT(rc);

        rc = out.Connect(api_.Endpoints().Shutdown().c_str());

        OT_ASSERT(rc);

        rc = out.Connect(api_.Endpoints().BlockchainReorg().c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , loopback_([&]() -> auto& {
        auto& out = batch_.sockets_.at(5);
        auto rc = out.Bind(loopback_endpoint_.c_str());

        OT_ASSERT(rc);

        LogTrace()(OT_PRETTY_CLASS())("loopback bound to ")(loopback_endpoint_)
            .Flush();

        return out;
    }())
    , to_loopback_([&] {
        const auto& context = api_.Network().ZeroMQ();
        auto socket = factory::ZMQSocket(context, SocketType::Pair);
        auto rc = socket.Connect(loopback_endpoint_.c_str());

        OT_ASSERT(rc);

        rc = socket.SetOutgoingHWM(0);

        OT_ASSERT(rc);

        return socket;
    }())
    , rd_()
    , eng_(rd_())
    , blank_()
    , timer_(api_.Network().Asio().Internal().GetTimer())
    , progress_()
    , servers_()
    , clients_()
    , providers_()
    , active_([&] {
        auto out = ActiveMap{};

        for (const auto& chain : opentxs::blockchain::DefinedChains()) {
            out.try_emplace(chain);
        }

        return out;
    }())
    , connected_servers_()
    , connected_count_(0)
    , running_(true)
    , thread_(api_.Network().ZeroMQ().Internal().Start(
          batch_.id_,
          {
              {external_router_.ID(),
               &external_router_,
               [id = external_router_.ID(), &cb = external_cb_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {monitor_.ID(),
               &monitor_,
               [id = monitor_.ID(), &cb = monitor_cb_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {external_sub_.ID(),
               &external_sub_,
               [id = external_sub_.ID(), &cb = external_cb_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {internal_router_.ID(),
               &internal_router_,
               [id = internal_router_.ID(), &cb = internal_cb_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {internal_sub_.ID(),
               &internal_sub_,
               [id = internal_sub_.ID(), &cb = internal_cb_](auto&& m) {
                   cb.Process(std::move(m));
               }},
              {loopback_.ID(),
               &loopback_,
               [id = loopback_.ID(), &cb = internal_cb_](auto&& m) {
                   cb.Process(std::move(m));
               }},
          }))
{
    OT_ASSERT(nullptr != thread_);
}

auto SyncClientRouter::Imp::Endpoint() const noexcept -> std::string_view
{
    return endpoint_;
}

auto SyncClientRouter::Imp::get_chain(Chain chain) const noexcept -> CString
{
    try {

        return clients_.at(chain);
    } catch (...) {

        return {};
    }
}

auto SyncClientRouter::Imp::get_provider(Chain chain) const noexcept -> CString
{
    try {
        const auto& providers = providers_.at(chain);
        auto result = UnallocatedVector<CString>{};
        std::sample(
            providers.begin(),
            providers.end(),
            std::back_inserter(result),
            1,
            eng_);

        if (0 < result.size()) {

            return result.front();
        } else {

            return {};
        }
    } catch (...) {

        return {};
    }
}

auto SyncClientRouter::Imp::get_required_height(Chain chain) const noexcept
    -> Height
{
    try {

        return progress_.at(chain);
    } catch (...) {

        return 0;
    }
}

auto SyncClientRouter::Imp::Init(const Blockchain& parent) noexcept -> void
{
    startup(parent);
    reset_timer();
}

auto SyncClientRouter::Imp::ping_server(
    syncclientrouter::Server& server) noexcept -> void
{
    external_router_.Send([&] {
        auto msg = opentxs::network::zeromq::Message{};
        msg.AddFrame(server.endpoint_);
        msg.StartBody();
        const auto query = factory::BlockchainSyncQuery(0);

        if (false == query.Serialize(msg)) { OT_FAIL; }

        return msg;
    }());
    server.last_sent_ = Clock::now();
    server.waiting_ = true;
}

auto SyncClientRouter::Imp::process_external(Message&& msg) noexcept -> void
{
    try {
        const auto body = msg.Body();

        if (0 == body.size()) {
            throw std::runtime_error{"Empty message body"};
        }

        const auto task = body.at(0).as<Task>();

        switch (task) {
            case Task::Ack:
            case Task::Reply:
            case Task::Push: {
            } break;
            case Task::Shutdown:
            case Task::Server:
            case Task::Register:
            case Task::Request:
            case Task::Processed:
            default: {
                const auto error =
                    CString{} + "Unsupported task on external socket: " +
                    std::to_string(static_cast<OTZMQWorkType>(task)).c_str();

                throw std::runtime_error{error.c_str()};
            }
        }

        const auto endpoint = CString{msg.at(0).Bytes()};
        auto& server = [&]() -> auto&
        {
            try {
                auto& out = servers_.at(endpoint);
                server_is_active(out);

                return out;
            } catch (...) {
                // NOTE blank_ gets returned for messages that arrive on a
                // subscribe socket rather than the router socket

                return blank_;
            }
        }
        ();
        const auto sync = api_.Factory().BlockchainSyncMessage(msg);
        const auto type = sync->Type();
        using Type = opentxs::network::p2p::MessageType;

        switch (type) {
            case Type::sync_ack: {
                const auto& ack = sync->asAcknowledgement();
                const auto& ep = ack.Endpoint();

                if (server.publisher_.empty() && (false == ep.empty())) {
                    if (external_sub_.Connect(ep.c_str())) {
                        server.publisher_ = ep;
                        LogDetail()("Subscribed to ")(
                            ep)(" for new block notifications")
                            .Flush();
                    } else {
                        LogError()(OT_PRETTY_CLASS())(
                            "failed to connect external subscriber to ")(ep)
                            .Flush();
                    }
                }

                for (const auto& state : ack.State()) {
                    const auto chain = state.Chain();
                    const auto height = server.ProcessState(state);
                    const auto acceptable =
                        height >= get_required_height(chain);
                    auto& providers = providers_[chain];

                    if (acceptable) {
                        providers.emplace(endpoint);
                        LogVerbose()(endpoint)(" has data for ")(
                            DisplayString(chain))
                            .Flush();
                    } else {
                        providers.erase(endpoint);
                        LogVerbose()(endpoint)(" is out of date for ")(
                            DisplayString(chain))
                            .Flush();
                    }

                    active_.at(chain).store(providers.size());
                    const auto& client = get_chain(chain);

                    if (client.empty()) { continue; }

                    namespace bcsync = opentxs::network::p2p;
                    auto data = [&] {
                        auto out = bcsync::StateData{};
                        const auto proto = [&] {
                            auto out = proto::BlockchainP2PChainState{};
                            state.Serialize(out);

                            return out;
                        }();

                        out.emplace_back(api_, proto);

                        return out;
                    }();
                    const auto notification =
                        factory::BlockchainSyncAcknowledgement(
                            std::move(data), ep);
                    auto msg = opentxs::network::zeromq::Message{};
                    msg.AddFrame(client);
                    msg.StartBody();

                    if (false == notification.Serialize(msg)) { OT_FAIL; }

                    internal_router_.Send(std::move(msg));
                }
            } break;
            case Type::new_block_header:
            case Type::sync_reply: {
                const auto& data = sync->asData();
                const auto chain = data.State().Chain();
                const auto identity = get_chain(chain);

                if (identity.empty()) {
                    const auto error = CString{} + "No active clients for " +
                                       DisplayString(chain).c_str();

                    throw std::runtime_error{error.c_str()};
                }

                auto msg = opentxs::network::zeromq::Message{};
                msg.AddFrame(identity);
                msg.StartBody();

                if (false == data.Serialize(msg)) { OT_FAIL; }

                internal_router_.Send(std::move(msg));
            } break;
            default: {
                const auto error =
                    CString{} +
                    "Unsupported message type on external socket: " +
                    opentxs::print(type).c_str();

                throw std::runtime_error{error.c_str()};
            }
        }
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return;
    }
}

auto SyncClientRouter::Imp::process_header(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();
    const auto index = [&]() -> std::size_t {
        if (5u < body.size()) {

            return 5;
        } else if (3u < body.size()) {

            return 3;
        } else {
            OT_FAIL;
        }
    }();
    const auto chain = body.at(1).as<Chain>();
    auto& height = progress_[chain];
    height = body.at(index).as<Height>();
    LogVerbose()(OT_PRETTY_CLASS())("minimum acceptable height for ")(
        DisplayString(chain))(" is ")(height)
        .Flush();
}

auto SyncClientRouter::Imp::process_internal(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (0 == body.size()) { OT_FAIL; }

    const auto type = body.at(0).as<Task>();

    switch (type) {
        case Task::Shutdown: {
            shutdown();
        } break;
        case Task::BlockHeader:
        case Task::Reorg: {
            process_header(std::move(msg));
        } break;
        case Task::Server: {
            process_server(std::move(msg));
        } break;
        case Task::Register: {
            process_register(std::move(msg));
        } break;
        case Task::Request: {
            process_request(std::move(msg));
        } break;
        case Task::StateMachine: {
            state_machine();
        } break;
        case Task::Ack:
        case Task::Reply:
        case Task::Push:
        case Task::Processed:
        default: {
            LogError()(OT_PRETTY_CLASS())(
                "Unsupported message type on internal socket: ")(
                static_cast<OTZMQWorkType>(type))
                .Flush();

            OT_FAIL;
        }
    }
}

auto SyncClientRouter::Imp::process_monitor(Message&& msg) noexcept -> void
{
    OT_ASSERT(2u == msg.size());

    const auto event = [&] {
        auto out = int{};
        auto& frame = msg.at(0);

        OT_ASSERT(2u <= frame.size());

        std::memcpy(&out, frame.data(), 2u);

        return out;
    }();

    switch (event) {
        case ZMQ_EVENT_HANDSHAKE_SUCCEEDED: {
            const auto endpoint = CString{msg.at(1).Bytes()};
            LogConsole()("Connected to p2p server at ")(endpoint).Flush();
            auto& server =
                servers_.try_emplace(endpoint, endpoint).first->second;
            server.connected_ = true;
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unexpected event type: ")(event)
                .Flush();

            OT_FAIL;
        }
    }
}

auto SyncClientRouter::Imp::process_register(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();
    const auto& identity = msg.at(0);

    OT_ASSERT(1 < body.size());

    const auto chain = body.at(1).as<Chain>();
    clients_[chain] = identity.Bytes();
    const auto& providers = providers_[chain];
    LogVerbose()(OT_PRETTY_CLASS())("querying ")(providers.size())(
        " providers for ")(DisplayString(chain))
        .Flush();

    for (const auto& endpoint : providers) {
        auto& server = servers_.at(endpoint);
        server.new_local_handler_ = true;
    }
}

auto SyncClientRouter::Imp::process_request(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    const auto chain = body.at(1).as<Chain>();
    const auto provider = get_provider(chain);

    if (provider.empty()) {
        LogError()(OT_PRETTY_CLASS())("no provider for ")(DisplayString(chain))
            .Flush();

        return;
    }

    auto& server = servers_.at(provider);
    const auto request = [&] {
        namespace otsync = opentxs::network::p2p;
        const auto proto =
            proto::Factory<proto::BlockchainP2PChainState>(body.at(2));
        auto states = [&] {
            auto out = otsync::StateData{};
            out.emplace_back(api_, proto);

            return out;
        }();

        return factory::BlockchainSyncRequest(std::move(states));
    }();
    external_router_.Send([&] {
        auto out = opentxs::network::zeromq::Message{};
        out.AddFrame(provider);
        out.StartBody();

        if (false == request.Serialize(out)) { OT_FAIL; }

        return out;
    }());
    server.last_sent_ = Clock::now();
}

auto SyncClientRouter::Imp::process_server(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    const auto added = body.at(2).as<bool>();

    if (added) {
        // TODO limit the number of active endpoints in an
        // intelligent way, attempting to ensure that all
        // blockchains have at least one endpoint that can provide
        // sync data
        process_server(CString{body.at(1).Bytes()});
    } else {
        // TODO decide whether or not to remove this server based on
        // criteria such as whether or not this endpoint is the only
        // provider of sync data for a particular chain
    }
}

auto SyncClientRouter::Imp::process_server(const CString ep) noexcept -> void
{
    if (external_router_.Connect(ep.c_str())) {
        servers_.try_emplace(ep, ep);
        LogDetail()(OT_PRETTY_CLASS())("Connecting to p2p server at ")(ep)
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to connect router to ")(ep)
            .Flush();

        return;
    }
}

auto SyncClientRouter::Imp::reset_timer() noexcept -> void
{
    static constexpr auto interval = std::chrono::seconds{10};
    timer_.SetRelative(interval);
    timer_.Wait([this](const auto& error) {
        if (error) {
            if (boost::system::errc::operation_canceled != error.value()) {
                LogError()(OT_PRETTY_CLASS())(error).Flush();
            }
        } else {
            to_loopback_.modify_detach([&](auto& socket) {
                socket.Send(MakeWork(Task::StateMachine));
            });
            reset_timer();
        }
    });
}

auto SyncClientRouter::Imp::server_is_active(
    syncclientrouter::Server& server) noexcept -> void
{
    if (server.endpoint_.empty()) { return; }

    server.active_ = true;
    server.last_received_ = Clock::now();
    server.waiting_ = false;
    connected_servers_.emplace(server.endpoint_);
    connected_count_.store(connected_servers_.size());
}

auto SyncClientRouter::Imp::server_is_stalled(
    syncclientrouter::Server& server) noexcept -> void
{
    connected_servers_.erase(server.endpoint_);
    connected_count_.store(connected_servers_.size());

    for (const auto& chain : server.Chains()) {
        auto& map = providers_[chain];
        map.erase(server.endpoint_);
        active_.at(chain).store(map.size());
    }

    if (false == server.publisher_.empty()) {
        external_sub_.Disconnect(server.publisher_.c_str());
        server.publisher_.clear();
    }

    server.SetStalled();
}

auto SyncClientRouter::Imp::shutdown() noexcept -> void
{
    if (auto run = running_.exchange(false); run) { batch_.ClearCallbacks(); }
}

auto SyncClientRouter::Imp::startup(const Blockchain& parent) noexcept -> void
{
    to_loopback_.modify_detach([&](auto& socket) {
        for (const auto& ep : parent.GetSyncServers()) {
            socket.Send([&] {
                auto out = MakeWork(Task::Server);
                out.AddFrame(ep);
                out.AddFrame(true);

                return out;
            }());
        }
    });
}

auto SyncClientRouter::Imp::state_machine() noexcept -> void
{
    for (auto& serverData : servers_) {
        auto& [endpoint, server] = serverData;

        if (server.connected_ && server.needs_query()) { ping_server(server); }

        if (server.is_stalled()) {
            if (server.active_) {
                server_is_stalled(server);
            } else if (server.needs_retry()) {
                ping_server(server);
            }
        }
    }
}

SyncClientRouter::Imp::~Imp()
{
    shutdown();
    api_.Network().ZeroMQ().Internal().Stop(batch_.id_);
}
}  // namespace opentxs::api::network::blockchain

namespace opentxs::api::network::blockchain
{
SyncClientRouter::SyncClientRouter(const api::Session& api) noexcept
    : SyncClientRouter(api, api.Network().ZeroMQ().Internal().MakeBatch([] {
        using Type = Imp::SocketType;
        auto out = UnallocatedVector<Type>{
            Type::Router,
            Type::Pair,
            Type::Subscribe,
            Type::Router,
            Type::Subscribe,
            Type::Pair,
        };

        return out;
    }()))
{
}

SyncClientRouter::SyncClientRouter(
    const api::Session& api,
    opentxs::network::zeromq::internal::Batch& batch) noexcept
    : imp_(std::make_unique<Imp>(api, batch).release())
{
}

auto SyncClientRouter::Endpoint() const noexcept -> std::string_view
{
    return imp_->Endpoint();
}

auto SyncClientRouter::Init(const Blockchain& parent) noexcept -> void
{
    return imp_->Init(parent);
}

SyncClientRouter::~SyncClientRouter()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::api::network::blockchain
