// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"

namespace opentxs::blockchain::client::implementation
{
using FilterDM = download::Manager<
    FilterOracle::FilterDownloader,
    std::unique_ptr<const GCS>,
    filter::pHeader,
    filter::Type>;
using FilterWorker = Worker<FilterOracle::FilterDownloader, api::Core>;

class FilterOracle::FilterDownloader : public FilterDM, public FilterWorker
{
public:
    auto Heartbeat() noexcept -> void
    {
        UpdatePosition(db_.FilterHeaderTip(type_));
        trigger();
    }
    auto NextBatch() noexcept { return allocate_batch(type_); }
    auto Start() noexcept { init_promise_.set_value(); }
    auto UpdatePosition(const Position& pos) -> void
    {
        try {
            auto current = known();
            auto hashes = header_.Ancestors(current, pos);

            OT_ASSERT(0 < hashes.size());

            auto prior = Previous{std::nullopt};
            {
                auto& first = hashes.front();

                if (first != current) {
                    auto promise = std::promise<filter::pHeader>{};
                    promise.set_value(
                        db_.LoadFilterHeader(type_, first.second->Bytes()));
                    prior.emplace(std::move(current), promise.get_future());
                }
            }
            hashes.erase(hashes.begin());
            update_position(std::move(hashes), type_, std::move(prior));
        } catch (...) {
        }
    }

    FilterDownloader(
        const api::Core& api,
        const internal::FilterDatabase& db,
        const internal::HeaderOracle& header,
        const internal::Network& network,
        const blockchain::Type chain,
        const filter::Type type,
        const std::string& shutdown) noexcept
        : FilterDM(
              [&] { return db.FilterTip(type); }(),
              [&] {
                  auto promise = std::promise<filter::pHeader>{};
                  const auto tip = db.FilterTip(type);
                  promise.set_value(
                      db.LoadFilterHeader(type, tip.second->Bytes()));

                  return Finished{promise.get_future()};
              }(),
              "cfilter")
        , FilterWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , network_(network)
        , chain_(chain)
        , type_(type)
        , socket_(api_.ZeroMQ().PublishSocket())
        , init_promise_()
        , init_(init_promise_.get_future())
    {
        init_executor({shutdown});
        auto zmq = socket_->Start(
            api_.Endpoints().InternalBlockchainFilterUpdated(chain_));

        OT_ASSERT(zmq);
    }

    ~FilterDownloader() { stop_worker().get(); }

private:
    friend FilterDM;
    friend FilterWorker;

    const internal::FilterDatabase& db_;
    const internal::HeaderOracle& header_;
    const internal::Network& network_;
    const blockchain::Type chain_;
    const filter::Type type_;
    OTZMQPublishSocket socket_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    auto batch_ready() const noexcept -> void
    {
        network_.JobReady(internal::PeerManager::Task::JobAvailableCfilters);
    }
    auto batch_size(std::size_t in) const noexcept -> std::size_t
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
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const filter::pHeader&)
        const noexcept -> void
    {
        const auto saved = db_.SetFilterTip(type_, position);

        OT_ASSERT(saved);

        LogVerbose(DisplayString(chain_))(" cfilter chain updated to height ")(
            position.first)
            .Flush();
        auto work = MakeWork(OT_ZMQ_NEW_FILTER_SIGNAL);
        work->AddFrame(type_);
        work->AddFrame(position.first);
        work->AddFrame(position.second);
        socket_->Send(work);
    }

    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        init_.get();

        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(0 < body.size());

        using Work = FilterOracle::Work;
        const auto work = body.at(0).as<Work>();

        switch (work) {
            case Work::reset_filter_tip: {
                process_reset(in);
            } break;
            case Work::statemachine: {
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
    auto process_reset(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(3 < body.size());

        auto position = Position{
            body.at(1).as<block::Height>(), api_.Factory().Data(body.at(2))};
        auto promise = std::promise<filter::pHeader>{};
        promise.set_value(api_.Factory().Data(body.at(3)));
        Reset(position, promise.get_future());
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        auto filters = std::vector<internal::FilterDatabase::Filter>{};

        for (const auto& task : data) {
            const auto& prior = task->previous_.get();
            auto& gcs =
                const_cast<std::unique_ptr<const GCS>&>(task->data_.get());
            const auto block = task->position_.second->Bytes();
            const auto expected = db_.LoadFilterHash(type_, block);

            if (expected == gcs->Hash()) {
                task->process(gcs->Header(prior->Bytes()));
                filters.emplace_back(block, gcs.release());
            } else {
                LogOutput("Filter for block ")(task->position_.second->asHex())(
                    " at height ")(task->position_.first)(
                    " does not match header. Received: ")(gcs->Hash()->asHex())(
                    " expected: ")(expected->asHex())
                    .Flush();
                task->redownload();
            }
        }

        const auto saved = db_.StoreFilters(type_, std::move(filters));

        OT_ASSERT(saved);
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
};
}  // namespace opentxs::blockchain::client::implementation
