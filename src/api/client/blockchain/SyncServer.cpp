// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "api/client/blockchain/SyncServer.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <chrono>
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

#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/BlockchainP2PChainState.pb.h"
#include "opentxs/protobuf/BlockchainP2PHello.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PHello.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::api::client::blockchain::SyncServer::Imp::"

namespace opentxs::api::client::blockchain
{
struct SyncServer::Imp {
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using Map = std::map<Chain, std::tuple<std::string, bool, Socket>>;
    using OTSocket = opentxs::network::zeromq::socket::implementation::Socket;

    const api::Core& api_;
    Blockchain& parent_;
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

        while (running_) {
            constexpr auto timeout = std::chrono::milliseconds{10};
            const auto events =
                ::zmq_poll(poll.data(), poll.size(), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogOutput(OT_METHOD)(__FUNCTION__)(": ")(::zmq_strerror(error))
                    .Flush();

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

    Imp(const api::Core& api, Blockchain& parent) noexcept
        : api_(api)
        , parent_(parent)
        , linger_(0)
        , sync_(::zmq_socket(api_.ZeroMQ(), ZMQ_ROUTER), ::zmq_close)
        , update_(::zmq_socket(api_.ZeroMQ(), ZMQ_PUB), ::zmq_close)
        , map_([&] {
            auto output = Map{};

            for (const auto chain : opentxs::blockchain::DefinedChains()) {
                auto& [endpoint, enabled, socket] =
                    output
                        .emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(chain),
                            std::forward_as_tuple(
                                OTSocket::random_inproc_endpoint(),
                                false,
                                Socket{
                                    ::zmq_socket(api_.ZeroMQ(), ZMQ_PAIR),
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
        const auto incoming = [&] {
            auto output = api_.ZeroMQ().Message();
            OTSocket::receive_message(lock, socket, output);

            return output;
        }();
        const auto body = incoming->Body();

        if (2 > body.size()) { return; }

        try {
            const auto type = body.at(0).as<WorkType>();

            switch (type) {
                case WorkType::SyncRequest: {
                    break;
                }
                default: {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Unsupported message type ")(value(type))
                        .Flush();

                    return;
                }
            }

            const auto hello =
                proto::Factory<proto::BlockchainP2PHello>(body.at(1));

            if (false == proto::Validate(hello, VERBOSE)) { return; }

            {
                auto outgoing = api_.ZeroMQ().TaggedReply(
                    incoming, WorkType::SyncAcknowledgement);
                outgoing->AddFrame(parent_.Hello());
                outgoing->AddFrame(update_public_endpoint_);
                OTSocket::send_message(lock, socket, outgoing);
            }

            for (const auto& state : hello.state()) {
                const auto chain = static_cast<Chain>(state.chain());

                try {
                    const auto& [endpoint, enabled, internal] = map_.at(chain);

                    if (enabled) {
                        OTSocket::send_message(
                            lock, internal.get(), OTZMQMessage{incoming});
                    }
                } catch (...) {
                }
            }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
        }
    }
    auto process_internal(const Lock& lock, void* socket) noexcept -> void
    {
        auto incoming = api_.ZeroMQ().Message();
        OTSocket::receive_message(lock, socket, incoming);
        const auto hSize = incoming->Header().size();
        const auto bSize = incoming->Body().size();

        if ((0u == hSize) && (0u == bSize)) { return; }

        auto* outgoing = (0u < hSize) ? sync_.get() : update_.get();
        OTSocket::send_message(lock, outgoing, incoming);
    }
};

SyncServer::SyncServer(const api::Core& api, Blockchain& parent) noexcept
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
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid endpoint").Flush();

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
        if (0 != ::zmq_bind(imp_.sync_.get(), sync.c_str())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": failed to bind sync endpoint to ")(sync)
                .Flush();

            return false;
        }

        if (0 != ::zmq_bind(imp_.update_.get(), update.c_str())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": failed to bind update endpoint to ")(update)
                .Flush();

            return false;
        }

        LogNormal("Blockchain sync server listening on endpoints ")(sync)(
            " and ")(update)
            .Flush();
        output = true;
        imp_.running_ = output;
        imp_.thread_ = std::thread{&Imp::thread, imp_p_.get()};
    }

    return output;
}

SyncServer::~SyncServer() = default;
}  // namespace opentxs::api::client::blockchain
