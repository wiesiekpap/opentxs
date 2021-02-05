// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <functional>

#include "blockchain/DownloadManager.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "util/ScopeGuard.hpp"

#define INDEXER                                                                \
    "opentxs::blockchain::client::implementation::FilterOracle::BlockIndexer:" \
    ":"

namespace opentxs::blockchain::client::implementation
{
using BlockDM = download::Manager<
    FilterOracle::BlockIndexer,
    std::shared_ptr<const block::bitcoin::Block>,
    filter::pHeader,
    filter::Type>;
using BlockWorker = Worker<FilterOracle::BlockIndexer, api::Core>;

class FilterOracle::BlockIndexer : public BlockDM, public BlockWorker
{
public:
    using Callback = std::function<std::unique_ptr<const GCS>(
        const filter::Type type,
        const block::bitcoin::Block& block)>;

    auto NextBatch() noexcept { return allocate_batch(type_); }

    BlockIndexer(
        const api::Core& api,
        const internal::FilterDatabase& db,
        const internal::HeaderOracle& header,
        const internal::BlockOracle& block,
        const internal::Network& network,
        const blockchain::Type chain,
        const filter::Type type,
        const std::string& shutdown,
        const NotifyCallback& notify,
        Callback cb) noexcept
        : BlockDM(
              [&] { return db.FilterTip(type); }(),
              [&] {
                  auto promise = std::promise<filter::pHeader>{};
                  const auto tip = db.FilterTip(type);
                  promise.set_value(
                      db.LoadFilterHeader(type, tip.second->Bytes()));

                  return Finished{promise.get_future()};
              }(),
              "filter")
        , BlockWorker(api, std::chrono::milliseconds{20})
        , db_(db)
        , header_(header)
        , block_(block)
        , network_(network)
        , chain_(chain)
        , type_(type)
        , cb_(std::move(cb))
        , notify_(notify)
    {
        init_executor(
            {shutdown,
             api_.Endpoints().InternalBlockchainBlockUpdated(chain_)});
        OT_ASSERT(cb_);
    }

    ~BlockIndexer() { stop_worker().get(); }

private:
    friend BlockDM;
    friend BlockWorker;

    const internal::FilterDatabase& db_;
    const internal::HeaderOracle& header_;
    const internal::BlockOracle& block_;
    const internal::Network& network_;
    const blockchain::Type chain_;
    const filter::Type type_;
    const Callback cb_;
    const NotifyCallback& notify_;

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
    auto trigger_state_machine() const noexcept -> void { trigger(); }
    auto update_tip(const Position& position, const filter::pHeader&)
        const noexcept -> void
    {
        auto saved = db_.SetFilterTip(type_, position);

        OT_ASSERT(saved);

        saved = db_.SetFilterHeaderTip(type_, position);

        OT_ASSERT(saved);

        LogDetail(DisplayString(chain_))(
            " cfheader and cfilter chain updated to height ")(position.first)
            .Flush();
        notify_(type_, position);
    }

    auto download() noexcept -> void
    {
        auto work = NextBatch();
        constexpr auto none = std::chrono::seconds{0};

        for (const auto& task : work.data_) {
            const auto& hash = task->position_.second;
            auto future = block_.LoadBitcoin(hash);

            if (std::future_status::ready == future.wait_for(none)) {
                auto block = future.get();

                OT_ASSERT(block);
                OT_ASSERT(block->ID() == hash);

                task->download(std::move(block));
            }
        }
    }
    auto pipeline(const zmq::Message& in) noexcept -> void
    {
        if (false == running_.get()) { return; }

        const auto body = in.Body();

        OT_ASSERT(1 <= body.size());

        using Work = FilterOracle::Work;
        const auto work = body.at(0).as<Work>();

        switch (work) {
            case Work::shutdown: {
                shutdown(shutdown_promise_);
            } break;
            case Work::heartbeat: {
                process_position(block_.Tip());
                run_if_enabled();
            } break;
            case Work::full_block: {
                process_position(in);
                run_if_enabled();
            } break;
            case Work::statemachine: {
                download();
                run_if_enabled();
            } break;
            default: {
                OT_FAIL;
            }
        }
    }
    auto process_position(const zmq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() > 2);

        process_position(Position{
            body.at(1).as<block::Height>(), api_.Factory().Data(body.at(2))});
    }
    auto process_position(const Position& pos) noexcept -> void
    {
        const auto current = known();
        auto compare{current};
        LogTrace(INDEXER)(__FUNCTION__)(":  Current position: ")(current.first)(
            ",")(current.second->asHex())
            .Flush();
        LogTrace(INDEXER)(__FUNCTION__)(": Incoming position: ")(pos.first)(
            ",")(pos.second->asHex())
            .Flush();
        auto hashes = decltype(header_.Ancestors(current, pos)){};
        auto prior = Previous{std::nullopt};
        auto searching{true};

        while (searching) {
            try {
                hashes = header_.Ancestors(compare, pos);

                OT_ASSERT(0 < hashes.size());
            } catch (const std::exception& e) {
                LogOutput(INDEXER)(__FUNCTION__)(": ")(e.what()).Flush();
                reset_to_genesis();

                return;
            }

            auto postcondition =
                ScopeGuard{[&] { hashes.erase(hashes.begin()); }};
            auto& first = hashes.front();
            LogTrace(INDEXER)(__FUNCTION__)(":          Ancestor: ")(
                first.first)(",")(first.second->asHex())
                .Flush();

            if (first == pos) { return; }

            if (first == current) {
                searching = false;
                break;
            }

            auto header = db_.LoadFilterHeader(type_, first.second->Bytes());
            const auto filter = db_.LoadFilter(type_, first.second->Bytes());

            if (header->empty()) {
                LogOutput(INDEXER)(__FUNCTION__)(
                    ": Missing cfheader for block ")(first.first)(",")(
                    first.second->asHex())
                    .Flush();
            }

            if (!filter) {
                LogOutput(INDEXER)(__FUNCTION__)(
                    ": Missing cfilter for block ")(first.first)(",")(
                    first.second->asHex())
                    .Flush();
            }

            if (filter && (false == header->empty())) {
                auto promise = std::promise<filter::pHeader>{};
                promise.set_value(std::move(header));
                prior.emplace(std::move(first), promise.get_future());
                searching = false;
                break;
            }

            OT_ASSERT(0 < first.first);

            const auto height = first.first - 1;
            compare = block::Position{height, header_.BestHash(height)};
        }

        update_position(std::move(hashes), type_, std::move(prior));
    }
    auto queue_processing(DownloadedData&& data) noexcept -> void
    {
        if (0 == data.size()) { return; }

        const auto& tip = data.back();
        auto blockHashes = std::vector<block::pHash>{};
        auto filterHashes = std::vector<filter::pHash>{};
        auto filters = std::vector<internal::FilterDatabase::Filter>{};
        auto headers = std::vector<internal::FilterDatabase::Header>{};
        const auto count = data.size();
        blockHashes.reserve(count);
        filterHashes.reserve(count);
        filters.reserve(count);
        headers.reserve(count);

        for (const auto& task : data) {
            try {
                const auto& blockhash = task->position_.second.get();
                const auto& pBlock = task->data_.get();

                if (false == bool(pBlock)) {
                    throw std::runtime_error(
                        std::string{"failed to load block "} +
                        blockhash.asHex());
                }

                const auto& block = *pBlock;
                const auto& id = blockHashes.emplace_back(blockhash).get();
                const auto& pGCS =
                    filters.emplace_back(id.Bytes(), cb_(type_, block)).second;

                if (false == bool(pGCS)) {
                    throw std::runtime_error(
                        std::string{"failed to calculate gcs for "} +
                        id.asHex());
                }

                const auto& gcs = *pGCS;
                const auto& previousHeader = task->previous_.get().get();
                const auto prior = previousHeader.Bytes();
                const auto& filterHash = filterHashes.emplace_back(gcs.Hash());
                const auto& it = headers.emplace_back(
                    id, gcs.Header(prior), filterHash->Bytes());
                const auto& [bHash, header, fHash] = it;

                if (header->empty()) {
                    throw std::runtime_error(
                        std::string{"failed to calculate filter header for "} +
                        id.asHex());
                }

                task->process(filter::pHeader{header});
            } catch (...) {
                task->redownload();
                break;
            }
        }

        const auto stored =
            db_.StoreFilters(type_, headers, filters, tip->position_);

        if (false == stored) { OT_FAIL; }
    }
    auto reset_to_genesis() noexcept -> void
    {
        LogOutput(INDEXER)(__FUNCTION__)(": Performing full reset").Flush();
        static const auto genesis =
            block::Position{0, header_.GenesisBlockHash(chain_)};
        auto promise = std::promise<filter::pHeader>{};
        promise.set_value(db_.LoadFilterHeader(type_, genesis.second->Bytes()));
        Reset(genesis, promise.get_future());
    }
    auto shutdown(std::promise<void>& promise) noexcept -> void
    {
        if (running_->Off()) {
            try {
                promise.set_value();
            } catch (...) {
            }
        }
    }
};
}  // namespace opentxs::blockchain::client::implementation
