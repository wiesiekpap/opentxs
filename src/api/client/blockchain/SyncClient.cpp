// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "api/client/blockchain/SyncClient.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <array>
#include <atomic>
#include <chrono>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <thread>
#include <utility>

#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"
#include "opentxs/network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/Data.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/blockchain/sync/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "util/AsyncValue.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::api::client::blockchain::SyncClient::Imp::"

namespace bc = opentxs::blockchain;

namespace opentxs::api::client::blockchain
{
struct SyncClient::Imp {

    auto Heartbeat(SyncState data) const noexcept -> void
    {
        if (false == init_.value_.get()) { return; }

        auto msg = api_.ZeroMQ().Message();

        try {
            using Request = opentxs::network::blockchain::sync::Request;
            const auto request = Request{std::move(data)};

            if (false == request.Serialize(msg)) { return; }
        } catch (...) {
        }

        auto lock = Lock{lock_};
        OTSocket::send_message(lock, sync_.get(), msg);
    }
    auto IsActive(const Chain chain) const noexcept -> bool
    {
        auto lock = Lock{lock_};

        try {

            return have_sync_.at(chain);
        } catch (...) {

            return false;
        }
    }
    auto IsConnected() const noexcept -> bool
    {
        using Status = std::future_status;
        constexpr auto timeout = std::chrono::seconds{30};
        const auto& future = sync_connected_.value_;

        if (Status::ready == future.wait_for(timeout)) {

            return future.get();
        } else {

            return false;
        }
    }

    Imp(const api::Core& api,
        Blockchain& parent,
        const std::string& endpoint) noexcept
        : api_(api)
        , parent_(parent)
        , linger_(0)
        , sync_(::zmq_socket(api_.ZeroMQ(), ZMQ_DEALER), ::zmq_close)
        , update_(::zmq_socket(api_.ZeroMQ(), ZMQ_SUB), ::zmq_close)
        , sync_endpoint_(endpoint)
        , update_endpoint_()
        , lock_()
        , init_()
        , sync_connected_()
        , update_connected_(false)
        , have_sync_([&] {
            auto output = StatusMap{};

            for (const auto& chain : bc::SupportedChains()) {
                output.emplace(chain, false);
            }

            return output;
        }())
        , running_(true)
        , thread_(&Imp::thread, this)
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
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using OTSocket = opentxs::network::zeromq::socket::implementation::Socket;
    using StatusMap = std::map<Chain, bool>;

    const api::Core& api_;
    Blockchain& parent_;
    const int linger_;
    Socket sync_;
    Socket update_;
    const std::string sync_endpoint_;
    std::string update_endpoint_;
    mutable std::mutex lock_;
    AsyncValue<bool> init_;
    AsyncValue<bool> sync_connected_;
    bool update_connected_;
    StatusMap have_sync_;
    std::atomic_bool running_;
    std::thread thread_;

    auto process(void* socket) noexcept -> void
    {
        auto msg = [&] {
            auto lock = Lock{lock_};
            auto output = api_.ZeroMQ().Message();
            OTSocket::receive_message(lock, socket, output);

            return output;
        }();
        const auto sync =
            opentxs::network::blockchain::sync::Factory(api_, msg);
        const auto type = sync->Type();
        using Type = opentxs::network::blockchain::sync::MessageType;

        switch (type) {
            case Type::sync_ack: {
                const auto& ack = sync->asAcknowledgement();
                const auto& endpoint = ack.Endpoint();

                {
                    auto lock = Lock{lock_};

                    for (const auto& state : ack.State()) {
                        have_sync_[state.Chain()] = true;
                    }
                }

                if (update_endpoint_.empty() && (false == endpoint.empty())) {
                    update_endpoint_ = endpoint;
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Received update endpoint ")(update_endpoint_)
                        .Flush();
                }

                if ((false == update_connected_) &&
                    (false == update_endpoint_.empty())) {
                    update_connected_ =
                        (0 == ::zmq_connect(
                                  update_.get(), update_endpoint_.c_str()));

                    if (false == update_connected_) {
                        LogOutput(OT_METHOD)(__FUNCTION__)(
                            ": failed to connect update endpoint to ")(
                            update_endpoint_)
                            .Flush();
                    }
                }
            } break;
            case Type::sync_reply:
            case Type::new_block_header: {
                const auto& data = sync->asData();

                if (0 < data.Blocks().size()) {
                    parent_.ProcessSyncData(data, std::move(msg));
                }
            } break;
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Unsupported message type ")(opentxs::print(type))
                    .Flush();

                return;
            }
        }
    }
    auto thread() noexcept -> void
    {
        auto postcondition = ScopeGuard{[this] {
            init_.set(false);
            sync_connected_.set(false);
        }};

        if (sync_endpoint_.empty()) { return; }

        if (0 != ::zmq_connect(sync_.get(), sync_endpoint_.c_str())) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": failed to connect sync endpoint to ")(sync_endpoint_)
                .Flush();

            return;
        }

        init_.set(true);
        auto poll = [&] {
            auto output = std::array<::zmq_pollitem_t, 2>{};

            {
                auto& item = output.at(0);
                item.socket = sync_.get();
                item.events = ZMQ_POLLIN;
            }
            {
                auto& item = output.at(1);
                item.socket = update_.get();
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

            for (const auto& item : poll) {
                if (ZMQ_POLLIN != item.revents) { continue; }

                process(item.socket);
                sync_connected_.set(true);
            }
        }
    }
};

SyncClient::SyncClient(
    const api::Core& api,
    Blockchain& parent,
    const std::string& endpoint) noexcept
    : imp_p_(std::make_unique<Imp>(api, parent, endpoint))
    , imp_(*imp_p_)
{
}

auto SyncClient::Heartbeat(SyncState data) const noexcept -> void
{
    return imp_.Heartbeat(std::move(data));
}

auto SyncClient::IsActive(const Chain chain) const noexcept -> bool
{
    return imp_.IsActive(chain);
}

auto SyncClient::IsConnected() const noexcept -> bool
{
    return imp_.IsConnected();
}

SyncClient::~SyncClient() = default;
}  // namespace opentxs::api::client::blockchain
