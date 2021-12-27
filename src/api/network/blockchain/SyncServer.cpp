// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "api/network/blockchain/SyncServer.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <chrono>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "internal/api/network/Blockchain.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/network/blockchain/sync/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"
#include "opentxs/network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/PublishContract.hpp"
#include "opentxs/network/blockchain/sync/PublishContractReply.hpp"
#include "opentxs/network/blockchain/sync/QueryContract.hpp"
#include "opentxs/network/blockchain/sync/QueryContractReply.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/blockchain/sync/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::api::network::blockchain
{
struct SyncServer::Imp {
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using Map = std::map<Chain, std::tuple<std::string, bool, Socket>>;
    using OTSocket = opentxs::network::zeromq::socket::implementation::Socket;

    const api::Session& api_;
    internal::Blockchain& parent_;
    const int linger_;
    Socket sync_;
    Socket update_;
    Map map_;
    std::string sync_endpoint_;
    std::string sync_public_endpoint_;
    std::string update_endpoint_;
    std::string update_public_endpoint_;
    mutable std::mutex lock_;
    std::atomic_bool running_;
    std::thread thread_;

    auto thread() noexcept -> void
    {
        auto poll = [&] {
            auto output = std::vector<::zmq_pollitem_t>{};

            {
                auto& item = output.emplace_back();
                item.socket = sync_.get();
                item.events = ZMQ_POLLIN;
            }

            for (const auto& [key, value] : map_) {
                auto& item = output.emplace_back();
                item.socket = std::get<2>(value).get();
                item.events = ZMQ_POLLIN;
            }

            return output;
        }();

        OT_ASSERT(std::numeric_limits<int>::max() >= poll.size());

        while (running_) {
            constexpr auto timeout = std::chrono::milliseconds{10};
            const auto events = ::zmq_poll(
                poll.data(), static_cast<int>(poll.size()), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogError()(OT_PRETTY_CLASS())(::zmq_strerror(error)).Flush();

                continue;
            } else if (0 == events) {
                continue;
            }

            auto lock = Lock{lock_};
            auto counter{-1};

            for (const auto& item : poll) {
                ++counter;

                if (ZMQ_POLLIN != item.revents) { continue; }

                if (0 == counter) {
                    process_external(lock, item.socket);
                } else {
                    process_internal(lock, item.socket);
                }
            }
        }
    }

    Imp(const api::Session& api, internal::Blockchain& parent) noexcept
        : api_(api)
        , parent_(parent)
        , linger_(0)
        , sync_(::zmq_socket(api_.Network().ZeroMQ(), ZMQ_ROUTER), ::zmq_close)
        , update_(::zmq_socket(api_.Network().ZeroMQ(), ZMQ_PUB), ::zmq_close)
        , map_([&] {
            auto output = Map{};

            for (const auto chain : opentxs::blockchain::DefinedChains()) {
                auto& [endpoint, enabled, socket] =
                    output
                        .emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(chain),
                            std::forward_as_tuple(
                                opentxs::network::zeromq::MakeArbitraryInproc(),
                                false,
                                Socket{
                                    ::zmq_socket(
                                        api_.Network().ZeroMQ(), ZMQ_PAIR),
                                    ::zmq_close}))
                        .first->second;
                ::zmq_setsockopt(
                    socket.get(), ZMQ_LINGER, &linger_, sizeof(linger_));
                ::zmq_bind(socket.get(), endpoint.c_str());
            }

            return output;
        }())
        , sync_endpoint_()
        , sync_public_endpoint_()
        , update_endpoint_()
        , update_public_endpoint_()
        , lock_()
        , running_(false)
        , thread_()
    {
        ::zmq_setsockopt(sync_.get(), ZMQ_LINGER, &linger_, sizeof(linger_));
        ::zmq_setsockopt(update_.get(), ZMQ_LINGER, &linger_, sizeof(linger_));
    }

    ~Imp()
    {
        running_ = false;

        if (thread_.joinable()) { thread_.join(); }
    }

private:
    auto process_external(const Lock& lock, void* socket) noexcept -> void
    {
        try {
            auto incoming = [&] {
                auto output = opentxs::network::zeromq::Message{};
                OTSocket::receive_message(lock, socket, output);

                return output;
            }();
            namespace bcsync = opentxs::network::blockchain::sync;
            const auto base = api_.Factory().BlockchainSyncMessage(incoming);

            if (!base) {
                throw std::runtime_error{"failed to instantiate message"};
            }

            const auto type = base->Type();

            switch (type) {
                case bcsync::MessageType::query:
                case bcsync::MessageType::sync_request: {
                    process_sync(lock, socket, incoming, *base);
                } break;
                case bcsync::MessageType::contract_query: {
                    process_query_contract(
                        lock, socket, std::move(incoming), *base);
                } break;
                case bcsync::MessageType::publish_contract: {
                    process_publish_contract(
                        lock, socket, std::move(incoming), *base);
                } break;
                default: {
                    throw std::runtime_error{
                        std::string{"Unsupported message type "} +
                        opentxs::print(type)};
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }
    }
    auto process_internal(const Lock& lock, void* socket) noexcept -> void
    {
        auto incoming = opentxs::network::zeromq::Message{};
        OTSocket::receive_message(lock, socket, incoming);
        const auto hSize = incoming.Header().size();
        const auto bSize = incoming.Body().size();

        if ((0u == hSize) && (0u == bSize)) { return; }

        if (0u < hSize) {
            LogTrace()(OT_PRETTY_CLASS())("transmitting sync reply").Flush();
            OTSocket::send_message(lock, sync_.get(), std::move(incoming));
        } else {
            LogTrace()(OT_PRETTY_CLASS())("broadcasting push notification")
                .Flush();
            OTSocket::send_message(lock, update_.get(), std::move(incoming));
        }
    }
    auto process_query_contract(
        const Lock& lock,
        void* socket,
        opentxs::network::zeromq::Message&& incoming,
        const opentxs::network::blockchain::sync::Base& base) noexcept -> void
    {
        auto payload = [&] {
            const auto& id = base.asQueryContract().ID();

            try {
                const auto type = translate(id.Type());

                switch (type) {
                    case contract::Type::nym: {
                        const auto nymID =
                            api_.Factory().InternalSession().NymID(id);
                        const auto nym = api_.Wallet().Nym(nymID);

                        if (!nym) {
                            throw std::runtime_error{
                                std::string{"nym "} + nymID->str() +
                                " not found"};
                        }

                        return factory::BlockchainSyncQueryContractReply(*nym);
                    }
                    case contract::Type::notary: {
                        const auto notaryID =
                            api_.Factory().InternalSession().ServerID(id);

                        return factory::BlockchainSyncQueryContractReply(
                            api_.Wallet().Server(notaryID));
                    }
                    case contract::Type::unit: {
                        const auto unitID =
                            api_.Factory().InternalSession().UnitID(id);

                        return factory::BlockchainSyncQueryContractReply(
                            api_.Wallet().UnitDefinition(unitID));
                    }
                    default: {
                        throw std::runtime_error{
                            std::string{
                                "unsupported or unknown contract type: "} +
                            opentxs::print(type)};
                    }
                }
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return factory::BlockchainSyncQueryContractReply(id);
            }
        }();
        OTSocket::send_message(lock, socket, [&] {
            auto out =
                opentxs::network::zeromq::reply_to_message(std::move(incoming));
            payload.Serialize(out);

            return out;
        }());
    }
    auto process_publish_contract(
        const Lock& lock,
        void* socket,
        opentxs::network::zeromq::Message&& incoming,
        const opentxs::network::blockchain::sync::Base& base) noexcept -> void
    {
        const auto& contract = base.asPublishContract();
        const auto& id = contract.ID();
        auto payload = [&] {
            try {
                const auto type = contract.ContractType();

                switch (type) {
                    case contract::Type::nym: {
                        const auto nym = api_.Wallet().Nym(contract.Payload());

                        return (nym && nym->ID() == id);
                    }
                    case contract::Type::notary: {
                        const auto notary =
                            api_.Wallet().Server(contract.Payload());

                        return (notary->ID() == id);
                    }
                    case contract::Type::unit: {
                        const auto unit =
                            api_.Wallet().UnitDefinition(contract.Payload());

                        return (unit->ID() == id);
                    }
                    default: {
                        throw std::runtime_error{
                            std::string{
                                "unsupported or unknown contract type: "} +
                            opentxs::print(type)};
                    }
                }
            } catch (const std::exception& e) {
                LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

                return false;
            }
        }();
        OTSocket::send_message(lock, socket, [&] {
            auto out =
                opentxs::network::zeromq::reply_to_message(std::move(incoming));
            const auto msg =
                factory::BlockchainSyncPublishContractReply(id, payload);
            msg.Serialize(out);

            return out;
        }());
    }
    auto process_sync(
        const Lock& lock,
        void* socket,
        const opentxs::network::zeromq::Message& incoming,
        const opentxs::network::blockchain::sync::Base& base) noexcept -> void
    {
        try {
            {
                const auto ack = factory::BlockchainSyncAcknowledgement(
                    parent_.Hello(), update_public_endpoint_);
                auto msg = opentxs::network::zeromq::reply_to_message(incoming);

                if (ack.Serialize(msg)) {
                    OTSocket::send_message(lock, socket, std::move(msg));
                }
            }

            namespace bcsync = opentxs::network::blockchain::sync;
            const auto type = base.Type();

            if (bcsync::MessageType::sync_request == type) {
                const auto& request = base.asRequest();

                for (const auto& state : request.State()) {
                    try {
                        const auto& [endpoint, enabled, internal] =
                            map_.at(state.Chain());

                        if (enabled) {
                            OTSocket::send_message(
                                lock,
                                internal.get(),
                                opentxs::network::zeromq::Message{incoming});
                        }
                    } catch (...) {
                    }
                }
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }
    }
};

SyncServer::SyncServer(
    const api::Session& api,
    internal::Blockchain& parent) noexcept
    : imp_p_(std::make_unique<Imp>(api, parent))
    , imp_(*imp_p_)
{
}

auto SyncServer::Disable(const Chain chain) noexcept -> void
{
    try {
        auto lock = Lock{imp_.lock_};
        std::get<1>(imp_.map_.at(chain)) = false;
    } catch (...) {
    }
}

auto SyncServer::Enable(const Chain chain) noexcept -> void
{
    try {
        auto lock = Lock{imp_.lock_};
        std::get<1>(imp_.map_.at(chain)) = true;
    } catch (...) {
    }
}

auto SyncServer::Endpoint(const Chain chain) const noexcept -> std::string
{
    try {

        return std::get<0>(imp_.map_.at(chain));
    } catch (...) {

        return {};
    }
}

auto SyncServer::Start(
    const std::string& sync,
    const std::string& publicSync,
    const std::string& update,
    const std::string& publicUpdate) noexcept -> bool
{
    if (sync.empty() || update.empty()) {
        LogError()(OT_PRETTY_CLASS())("Invalid endpoint").Flush();

        return false;
    }

    auto lock = Lock{imp_.lock_};
    auto output{false};
    auto postcondition = ScopeGuard{[&] {
        if (output) {
            imp_.sync_endpoint_ = sync;
            imp_.sync_public_endpoint_ = publicSync;
            imp_.update_endpoint_ = update;
            imp_.update_public_endpoint_ = publicUpdate;
        } else {
            ::zmq_unbind(imp_.sync_.get(), sync.c_str());
            ::zmq_unbind(imp_.update_.get(), update.c_str());
        }
    }};

    if (false == imp_.running_) {
        if (0 == ::zmq_setsockopt(
                     imp_.sync_.get(),
                     ZMQ_ROUTING_ID,
                     publicSync.data(),
                     publicSync.size())) {
            LogDebug()("Sync socket identity set to public endpoint: ")(
                publicSync)
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to set sync socket identity")
                .Flush();

            return false;
        }

        if (0 == ::zmq_bind(imp_.sync_.get(), sync.c_str())) {
            LogConsole()("Blockchain sync server listener bound to ")(sync)
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to bind sync endpoint to ")(
                sync)
                .Flush();

            return false;
        }

        if (0 == ::zmq_bind(imp_.update_.get(), update.c_str())) {
            LogConsole()("Blockchain sync server publisher bound to ")(update)
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to bind update endpoint to ")(
                update)
                .Flush();

            return false;
        }

        output = true;
        imp_.running_ = output;
        imp_.thread_ = std::thread{&Imp::thread, imp_p_.get()};
    }

    return output;
}

SyncServer::~SyncServer() = default;
}  // namespace opentxs::api::network::blockchain
