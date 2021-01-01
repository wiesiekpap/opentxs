// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <functional>
#include <limits>
#include <mutex>
#include <thread>

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/protobuf/BlockchainP2PChainState.pb.h"
#include "opentxs/protobuf/BlockchainP2PHello.pb.h"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PHello.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::client::implementation
{
using SyncDM = download::
    Manager<Network::SyncServer, std::unique_ptr<const GCS>, int, filter::Type>;
using SyncWorker = Worker<Network::SyncServer, api::Core>;

class Network::SyncServer : public SyncDM, public SyncWorker
{
public:
    auto Heartbeat() noexcept -> void
    {
        process_position(filter_.Tip(type_));
        trigger();
    }
    auto NextBatch() noexcept { return allocate_batch(type_); }
    auto Start() noexcept { init_promise_.set_value(); }

    SyncServer(
        const api::Core& api,
        const internal::SyncDatabase& db,
        const internal::HeaderOracle& header,
        const internal::FilterOracle& filter,
        const internal::Network& network,
        const blockchain::Type chain,
        const filter::Type type,
        const std::string& shutdown,
        const std::string& publishEndpoint) noexcept
        : SyncDM(
              [&] { return db.SyncTip(); }(),
              [&] {
                  auto promise = std::promise<int>{};
                  promise.set_value(0);

                  return Finished{promise.get_future()};
              }(),
              "sync server")
        , SyncWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , filter_(filter)
        , network_(network)
        , chain_(chain)
        , type_(type)
        , linger_(0)
        , endpoint_(publishEndpoint)
        , socket_(::zmq_socket(api_.ZeroMQ(), ZMQ_PAIR), ::zmq_close)
        , zmq_lock_()
        , zmq_running_(true)
        , init_promise_()
        , init_(init_promise_.get_future())
        , zmq_thread_(&SyncServer::zmq_thread, this)
    {
        init_executor(
            {shutdown,
             api_.Endpoints().InternalBlockchainFilterUpdated(chain_)});
        ::zmq_setsockopt(socket_.get(), ZMQ_LINGER, &linger_, sizeof(linger_));
        ::zmq_connect(socket_.get(), endpoint_.c_str());
        init_promise_.set_value();
    }

    ~SyncServer()
    {
        {
            auto lock = Lock{zmq_lock_};
            zmq_running_ = false;
        }

        if (zmq_thread_.joinable()) { zmq_thread_.join(); }

        stop_worker().get();
    }

private:
    friend SyncDM;
    friend SyncWorker;

    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using OTSocket = opentxs::network::zeromq::socket::implementation::Socket;

    const internal::SyncDatabase& db_;
    const internal::HeaderOracle& header_;
    const internal::FilterOracle& filter_;
    const internal::Network& network_;
    const blockchain::Type chain_;
    const filter::Type type_;
    const int linger_;
    const std::string endpoint_;
    Socket socket_;
    mutable std::mutex zmq_lock_;
    std::atomic_bool zmq_running_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    std::thread zmq_thread_;

    auto batch_ready() const noexcept -> void { trigger(); }
    auto batch_size(const std::size_t in) const noexcept -> std::size_t
    {
        if (in < 10) {

            return 1;
        } else if (in < 100) {

            return 10;
        } else if (in < 1000) {

            return 100;
        } else {

            return 1000;
        }
    }
    auto check_task(TaskType&) const noexcept -> void {}
    auto hello(const Lock&, const block::Position& incoming) const noexcept
    {
        // TODO use known() and Ancestors() instead
        const auto [parent, best] = header_.CommonParent(incoming);
        auto proto = proto::BlockchainP2PHello{};
        proto.set_version(client::sync_hello_version_);
        auto& state = *proto.add_state();
        state.set_version(client::sync_state_version_);
        state.set_chain(static_cast<std::uint32_t>(chain_));
        state.set_height(static_cast<std::uint64_t>(best.first));
        const auto bytes = best.second->Bytes();
        state.set_hash(bytes.data(), bytes.size());

        return std::make_tuple(incoming != best, parent, proto);
    }
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const int&) const noexcept -> void
    {
        auto saved = db_.SetSyncTip(position);

        OT_ASSERT(saved);

        LogNormal(DisplayString(chain_))(" sync data updated to height ")(
            position.first)
            .Flush();
    }

    auto download() noexcept -> void
    {
        auto work = NextBatch();

        for (const auto& task : work.data_) {
            task->download(filter_.LoadFilter(type_, task->position_.second));
        }
    }
    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        init_.get();

        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(1 <= body.size());

        using Work = Network::Work;
        const auto work = body.at(0).as<Work>();
        auto lock = Lock{zmq_lock_};

        switch (work) {
            case Work::filter: {
                process_position(in);
                do_work();
            } break;
            case Work::statemachine: {
                download();
                do_work();
            } break;
            case Work::shutdown: {
                shutdown(shutdown_promise_);
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto process_position(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() > 3);

        const auto type = body.at(1).as<filter::Type>();

        if (type != type_) { return; }

        process_position(Position{
            body.at(2).as<block::Height>(), api_.Factory().Data(body.at(3))});
    }
    auto process_position(const Position& pos) noexcept -> void
    {
        try {
            auto current = known();
            auto hashes = header_.Ancestors(current, pos);

            OT_ASSERT(0 < hashes.size());

            auto prior = Previous{std::nullopt};
            {
                auto& first = hashes.front();

                if (first != current) {
                    auto promise = std::promise<int>{};
                    promise.set_value(0);
                    prior.emplace(std::move(first), promise.get_future());
                }
            }
            hashes.erase(hashes.begin());
            update_position(std::move(hashes), type_, std::move(prior));
        } catch (...) {
        }
    }
    auto process_zmq(const Lock& lock) noexcept -> void
    {
        const auto incoming = [&] {
            auto output = api_.ZeroMQ().Message();
            OTSocket::receive_message(lock, socket_.get(), output);

            return output;
        }();
        const auto body = incoming->Body();

        if (1 > body.size()) { return; }

        const auto hello =
            proto::Factory<proto::BlockchainP2PHello>(body.at(0));

        if (false == proto::Validate(hello, VERBOSE)) { return; }

        for (const auto& state : hello.state()) {
            const auto chain = static_cast<blockchain::Type>(state.chain());

            if (chain_ != chain) { continue; }

            const auto position = block::Position{
                static_cast<block::Height>(state.height()),
                api_.Factory().Data(state.hash(), StringStyle::Raw)};
            const auto [needSync, parent, hello] = this->hello(lock, position);
            const auto& [height, hash] = parent;
            auto reply = api_.ZeroMQ().ReplyMessage(incoming);
            reply->AddFrame(hello);
            reply->AddFrame();
            auto send{true};

            if (needSync) { send = db_.LoadSync(height, reply); }

            if (send) { OTSocket::send_message(lock, socket_.get(), reply); }
        }
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        const auto& tip = data.back();
        auto items = std::vector<proto::BlockchainP2PSync>{};

        for (const auto& task : data) {
            try {
                const auto pHeader =
                    header_.LoadBitcoinHeader(task->position_.second);

                if (false == bool(pHeader)) {
                    throw std::runtime_error(
                        std::string{"failed to load block header "} +
                        task->position_.second->asHex());
                }

                const auto& header = *pHeader;
                const auto& pGCS = task->data_.get();

                if (false == bool(pGCS)) {
                    throw std::runtime_error(
                        std::string{"failed to load gcs for block "} +
                        task->position_.second->asHex());
                }

                const auto& gcs = *pGCS;
                auto& item = items.emplace_back();
                item.set_version(sync_data_version_);
                item.set_chain(static_cast<std::uint32_t>(chain_));
                item.set_height(task->position_.first);
                item.set_header(header.Encode()->str());
                item.set_filter_type(static_cast<std::uint32_t>(type_));
                item.set_filter_element_count(gcs.ElementCount());
                item.set_filter(std::string{reader(gcs.Compressed())});
                task->process(1);
            } catch (const std::exception& e) {
                LogOutput("opentxs::blockchain::client::implementation::"
                          "Network::SyncServer::")(__FUNCTION__)(": ")(e.what())
                    .Flush();
                task->redownload();
                break;
            }
        }

        const auto& pos = tip->position_;
        const auto stored = db_.StoreSync(pos, items);

        if (false == stored) { OT_FAIL; }

        auto work = api_.ZeroMQ().Message();
        work->StartBody();
        auto hello = [&] {
            auto output = proto::BlockchainP2PHello{};
            output.set_version(sync_hello_version_);
            auto& state = *output.add_state();
            state.set_version(sync_state_version_);
            state.set_chain(static_cast<std::uint32_t>(chain_));
            state.set_height(static_cast<std::uint64_t>(pos.first));
            const auto bytes = pos.second->Bytes();
            state.set_hash(bytes.data(), bytes.size());

            return output;
        }();
        work->AddFrame(hello);

        for (const auto& item : items) { work->AddFrame(item); }

        {
            // NOTE the appropriate lock is already being held in the pipeline
            // function
            static std::mutex dummy{};

            if (zmq_running_) {
                auto lock = Lock{dummy};
                OTSocket::send_message(lock, socket_.get(), work);
            }
        }
    }
    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        init_.get();

        if (running_->Off()) {
            try {
                promise.set_value();
            } catch (...) {
            }
        }
    }
    auto zmq_thread() noexcept -> void
    {
        init_.get();
        auto poll = [&] {
            auto output = std::vector<::zmq_pollitem_t>{};

            {
                auto& item = output.emplace_back();
                item.socket = socket_.get();
                item.events = ZMQ_POLLIN;
            }

            return output;
        }();

        while (zmq_running_) {
            constexpr auto timeout = std::chrono::milliseconds{250};
            const auto events =
                ::zmq_poll(poll.data(), poll.size(), timeout.count());

            if (0 > events) {
                const auto error = ::zmq_errno();
                LogOutput("opentxs::blockchain::client::implementation::"
                          "Network::SyncServer::")(__FUNCTION__)(": ")(
                    ::zmq_strerror(error))
                    .Flush();

                continue;
            } else if (0 == events) {

                continue;
            }

            auto lock = Lock{zmq_lock_};

            for (const auto& item : poll) {
                if (ZMQ_POLLIN != item.revents) { continue; }

                process_zmq(lock);
            }
        }

        ::zmq_disconnect(socket_.get(), endpoint_.c_str());
    }
};
}  // namespace opentxs::blockchain::client::implementation
