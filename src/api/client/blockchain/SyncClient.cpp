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
#include <stdexcept>
#include <thread>
#include <utility>

#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
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
#include "util/AsyncValue.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::api::client::blockchain::SyncClient::Imp::"

namespace bc = opentxs::blockchain;

namespace opentxs::api::client::blockchain
{
struct SyncClient::Imp {

    auto Heartbeat(const proto::BlockchainP2PHello& data) const noexcept -> void
    {
        if (false == init_.value_.get()) { return; }

        auto msg = api_.ZeroMQ().TaggedMessage(WorkType::SyncRequest);
        msg->AddFrame(data);
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
        auto incoming = [&] {
            auto lock = Lock{lock_};
            auto output = api_.ZeroMQ().Message();
            OTSocket::receive_message(lock, socket, output);

            return output;
        }();
        const auto body = incoming->Body();

        try {
            if (3 > body.size()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

                return;
            }

            const auto type = [&] {
                try {

                    return body.at(0).as<WorkType>();
                } catch (...) {

                    OT_FAIL;
                }
            }();

            switch (type) {
                case WorkType::SyncAcknowledgement:
                case WorkType::SyncReply:
                case WorkType::NewBlock: {
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

            if (false == proto::Validate(hello, VERBOSE)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid hello").Flush();

                return;
            }

            const auto hasEndpoint = (WorkType::SyncAcknowledgement == type) &&
                                     (0 < body.at(2).size());
            const auto hasSyncData =
                (WorkType::SyncReply == type) && (3 < body.size());

            {
                auto lock = Lock{lock_};
                for (const auto& state : hello.state()) {
                    have_sync_[static_cast<Chain>(state.chain())] = true;
                }

                if (update_endpoint_.empty() && hasEndpoint) {
                    update_endpoint_ = body.at(2).Bytes();
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
            }

            if (hasSyncData) { parent_.ProcessSyncData(std::move(incoming)); }
        } catch (const std::exception& e) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();
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

auto SyncClient::Heartbeat(const proto::BlockchainP2PHello& data) const noexcept
    -> void
{
    return imp_.Heartbeat(data);
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
