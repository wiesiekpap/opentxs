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

#include "internal/api/client/Client.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"
#include "opentxs/network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/blockchain/sync/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::api::network::blockchain::SyncServer::Imp::"

namespace opentxs::api::network::blockchain
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

        OT_ASSERT(std::numeric_limits<int>::max() >= poll.size());

        while (running_) {
            constexpr auto timeout = std::chrono::milliseconds{10};
            const auto events = ::zmq_poll(
                poll.data(), static_cast<int>(poll.size()), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogOutput(OT_METHOD)(__func__)(": ")(::zmq_strerror(error))
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
                                OTSocket::random_inproc_endpoint(),
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
        const auto incoming = [&] {
            auto output = api_.Network().ZeroMQ().Message();
            OTSocket::receive_message(lock, socket, output);

            return output;
        }();

        try {
            namespace sync = opentxs::network::blockchain::sync;
            const auto base = sync::Factory(api_, incoming);
            const auto type = base->Type();

            switch (type) {
                case sync::MessageType::query:
                case sync::MessageType::sync_request: {
                } break;
                default: {
                    LogOutput(OT_METHOD)(__func__)(
                        ": Unsupported message type ")(opentxs::print(type))
                        .Flush();

                    return;
                }
            }

            {
                const auto ack = sync::Acknowledgement{
                    parent_.Hello(), update_public_endpoint_};
                auto msg = api_.Network().ZeroMQ().ReplyMessage(incoming);

                if (ack.Serialize(msg)) {
                    OTSocket::send_message(lock, socket, msg);
                }
            }

            if (sync::MessageType::sync_request == type) {
                const auto& request = base->asRequest();

                for (const auto& state : request.State()) {
                    try {
                        const auto& [endpoint, enabled, internal] =
                            map_.at(state.Chain());

                        if (enabled) {
                            OTSocket::send_message(
                                lock, internal.get(), OTZMQMessage{incoming});
                        }
                    } catch (...) {
                    }
                }
            }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();
        }
    }
    auto process_internal(const Lock& lock, void* socket) noexcept -> void
    {
        auto incoming = api_.Network().ZeroMQ().Message();
        OTSocket::receive_message(lock, socket, incoming);
        const auto hSize = incoming->Header().size();
        const auto bSize = incoming->Body().size();

        if ((0u == hSize) && (0u == bSize)) { return; }

        if (0u < hSize) {
            LogTrace(OT_METHOD)(__func__)(": transmitting sync reply").Flush();
            OTSocket::send_message(lock, sync_.get(), incoming);
        } else {
            LogTrace(OT_METHOD)(__func__)(": broadcasting push notification")
                .Flush();
            OTSocket::send_message(lock, update_.get(), incoming);
        }
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
        LogOutput(OT_METHOD)(__func__)(": Invalid endpoint").Flush();

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
            LogDebug("Sync socket identity set to public endpoint: ")(
                publicSync)
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__func__)(
                ": failed to set sync socket identity")
                .Flush();

            return false;
        }

        if (0 == ::zmq_bind(imp_.sync_.get(), sync.c_str())) {
            LogNormal("Blockchain sync server listener bound to ")(sync)
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__func__)(
                ": failed to bind sync endpoint to ")(sync)
                .Flush();

            return false;
        }

        if (0 == ::zmq_bind(imp_.update_.get(), update.c_str())) {
            LogNormal("Blockchain sync server publisher bound to ")(update)
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__func__)(
                ": failed to bind update endpoint to ")(update)
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
