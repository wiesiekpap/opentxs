// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/client/FilterOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/client/filteroracle/BlockIndexer.hpp"
#include "blockchain/client/filteroracle/FilterCheckpoints.hpp"
#include "blockchain/client/filteroracle/FilterDownloader.hpp"
#include "blockchain/client/filteroracle/HeaderDownloader.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/client/Factory.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/client/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::client::implementation::FilterOracle::"

using ReturnType = opentxs::blockchain::client::implementation::FilterOracle;

namespace opentxs::factory
{
auto BlockchainFilterOracle(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::client::internal::Config& config,
    const blockchain::client::internal::Network& network,
    const blockchain::client::internal::HeaderOracle& header,
    const blockchain::client::internal::BlockOracle& block,
    const blockchain::client::internal::FilterDatabase& database,
    const blockchain::Type type,
    const std::string& shutdown) noexcept
    -> std::unique_ptr<blockchain::client::internal::FilterOracle>
{
    return std::make_unique<ReturnType>(
        api,
        blockchain,
        config,
        network,
        header,
        block,
        database,
        type,
        shutdown);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::implementation
{
FilterOracle::FilterOracle(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Config& config,
    const internal::Network& network,
    const internal::HeaderOracle& header,
    const internal::BlockOracle& block,
    const internal::FilterDatabase& database,
    const blockchain::Type chain,
    const std::string& shutdown) noexcept
    : internal::FilterOracle()
    , api_(api)
    , network_(network)
    , header_(header)
    , database_(database)
    , chain_(chain)
    , default_type_([&] {
        if (config.generate_cfilters_ || config.use_sync_server_) {

            return filter::Type::Extended_opentxs;
        }

        return blockchain::internal::DefaultFilter(chain_);
    }())
    , lock_()
    , new_filters_(api_.ZeroMQ().PublishSocket())
    , cb_([this](const auto type, const auto& pos) { new_tip(type, pos); })
    , filter_downloader_([&]() -> std::unique_ptr<FilterDownloader> {
        if (config.download_cfilters_) {
            return std::make_unique<FilterDownloader>(
                api,
                database,
                header,
                network,
                chain,
                default_type_,
                shutdown,
                cb_);
        } else {
            return {};
        }
    }())
    , header_downloader_([&]() -> std::unique_ptr<HeaderDownloader> {
        if (config.download_cfilters_) {
            return std::make_unique<HeaderDownloader>(
                api,
                database,
                header,
                network,
                *filter_downloader_,
                chain,
                default_type_,
                shutdown,
                [&](const auto& position, const auto& header) {
                    return compare_header_to_checkpoint(position, header);
                });
        } else {
            return {};
        }
    }())
    , block_indexer_([&]() -> std::unique_ptr<BlockIndexer> {
        if (config.generate_cfilters_) {
            return std::make_unique<BlockIndexer>(
                api,
                database,
                header,
                block,
                network,
                chain,
                default_type_,
                shutdown,
                cb_,
                [&](const auto type, const auto& block) {
                    return process_block(type, block);
                });
        } else {
            return {};
        }
    }())
    , init_promise_()
    , shutdown_promise_()
    , init_(init_promise_.get_future())
    , shutdown_(shutdown_promise_.get_future())
{
    auto zmq = new_filters_->Start(
        api_.Endpoints().InternalBlockchainFilterUpdated(chain_));

    OT_ASSERT(zmq);
    OT_ASSERT(cb_);
}

auto FilterOracle::compare_header_to_checkpoint(
    const block::Position& block,
    const filter::Header& receivedHeader) noexcept -> block::Position
{
    const auto& cp = filter_checkpoints_.at(chain_);
    const auto height = block.first;

    if (auto it = cp.find(height); cp.end() != it) {
        const auto& bytes = it->second.at(default_type_);
        const auto expectedHeader =
            api_.Factory().Data(ReadView{bytes.data(), bytes.size()});

        if (expectedHeader == receivedHeader) {
            LogNormal(DisplayString(chain_))(" filter header at height ")(
                height)(" verified against checkpoint")
                .Flush();

            return block;
        } else {
            OT_ASSERT(cp.begin() != it);

            std::advance(it, -1);
            const auto rollback =
                block::Position{it->first, header_.BestHash(it->first)};
            LogNormal(DisplayString(chain_))(" filter header at height ")(
                height)(" does not match checkpoint. Resetting to previous "
                        "checkpoint at height ")(rollback.first)
                .Flush();

            return rollback;
        }
    }

    return block;
}

auto FilterOracle::compare_tips_to_checkpoint() noexcept -> void
{
    const auto& cp = filter_checkpoints_.at(chain_);
    const auto headerTip = database_.FilterHeaderTip(default_type_);
    auto checkPosition{headerTip};
    auto changed{false};

    for (auto i{cp.crbegin()}; i != cp.crend(); ++i) {
        const auto& cpHeight = i->first;

        if (cpHeight > checkPosition.first) { continue; }

        checkPosition = block::Position{cpHeight, header_.BestHash(cpHeight)};
        const auto existingHeader = database_.LoadFilterHeader(
            default_type_, checkPosition.second->Bytes());

        try {
            const auto& cpHeader = i->second.at(default_type_);
            const auto cpBytes =
                api_.Factory().Data(ReadView{cpHeader.data(), cpHeader.size()});

            if (existingHeader == cpBytes) { break; }

            changed = true;
        } catch (...) {
            break;
        }
    }

    if (changed) {
        LogNormal(DisplayString(chain_))(
            " filter header chain did not match checkpoint. Resetting to last "
            "known good position")
            .Flush();
        reset_tips_to(default_type_, headerTip, checkPosition, changed);
    } else {
        LogVerbose(DisplayString(chain_))(
            " filter header chain matched checkpoint")
            .Flush();
    }
}

auto FilterOracle::compare_tips_to_header_chain() noexcept -> bool
{
    const auto current = database_.FilterHeaderTip(default_type_);
    const auto [parent, best] = header_.CommonParent(current);

    if ((parent.first == current.first) && (parent.second == current.second)) {
        LogVerbose(DisplayString(chain_))(
            " filter header chain is following the best chain")
            .Flush();

        return false;
    }

    LogNormal(DisplayString(chain_))(
        " filter header chain is following a sibling chain. Resetting to "
        "common ancestor at height ")(parent.first)
        .Flush();
    reset_tips_to(default_type_, current, parent);

    return true;
}

auto FilterOracle::GetFilterJob() const noexcept -> CfilterJob
{
    auto lock = Lock{lock_};

    if (filter_downloader_) {

        return filter_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto FilterOracle::GetHeaderJob() const noexcept -> CfheaderJob
{
    auto lock = Lock{lock_};

    if (header_downloader_) {

        return header_downloader_->NextBatch();
    } else {

        return {};
    }
}

auto FilterOracle::Heartbeat() const noexcept -> void
{
    auto lock = Lock{lock_};

    if (filter_downloader_) { filter_downloader_->Heartbeat(); }
    if (header_downloader_) { header_downloader_->Heartbeat(); }
    if (block_indexer_) { block_indexer_->Heartbeat(); }
}

auto FilterOracle::LoadFilterOrResetTip(
    const filter::Type type,
    const block::Position& position) const noexcept
    -> std::unique_ptr<const GCS>
{
    auto output = LoadFilter(type, position.second);

    if (output) { return output; }

    const auto height = position.first;

    OT_ASSERT(0 < height);

    const auto parent = height - 1;
    const auto hash = header_.BestHash(parent);

    OT_ASSERT(false == hash->empty());

    reset_tips_to(type, block::Position{parent, hash}, false, true);

    return {};
}

auto FilterOracle::new_tip(const filter::Type type, const block::Position& tip)
    const noexcept -> void
{
    auto work = MakeWork(api_, OT_ZMQ_NEW_FILTER_SIGNAL);
    work->AddFrame(type);
    work->AddFrame(tip.first);
    work->AddFrame(tip.second);
    new_filters_->Send(work);
}

auto FilterOracle::ProcessBlock(
    const block::bitcoin::Block& block) const noexcept -> bool
{
    const auto& id = block.ID();
    const auto& header = block.Header();
    auto filters = std::vector<internal::FilterDatabase::Filter>{};
    auto headers = std::vector<internal::FilterDatabase::Header>{};
    const auto& pGCS =
        filters.emplace_back(id.Bytes(), process_block(default_type_, block))
            .second;

    if (false == bool(pGCS)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to calculate ")(
            DisplayString(chain_))(" cfilter")
            .Flush();

        return false;
    }

    const auto& gcs = *pGCS;
    const auto previousHeader =
        LoadFilterHeader(default_type_, header.ParentHash());

    if (previousHeader->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": failed to load previous")(
            DisplayString(chain_))(" cfheader")
            .Flush();

        return false;
    }

    const auto filterHash = gcs.Hash();
    const auto& cfheader = std::get<1>(headers.emplace_back(
        id, gcs.Header(previousHeader->Bytes()), filterHash->Bytes()));

    if (cfheader->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": failed to calculate ")(
            DisplayString(chain_))(" cfheader")
            .Flush();

        return false;
    }

    const auto position = make_blank<block::Position>::value(api_);
    const auto stored =
        database_.StoreFilters(default_type_, headers, filters, position);

    if (stored) {

        return true;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error ").Flush();

        return false;
    }
}

auto FilterOracle::ProcessSyncData(const ParsedSyncData& parsed) const noexcept
    -> void
{
    const auto& [prior, data] = parsed;

    OT_ASSERT(0 < data.size());

    auto filters = std::vector<internal::FilterDatabase::Filter>{};
    auto headers = std::vector<internal::FilterDatabase::Header>{};
    const auto filterType = std::get<2>(data.front());

    try {
        auto previous = [&] {
            if (parsed.first->empty()) {

                return block::BlankHash();
            } else {

                auto output = LoadFilterHeader(filterType, parsed.first);

                if (output->empty()) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(": cfheader for ")(
                        DisplayString(chain_))(" block ")(
                        parsed.first->asHex())(" not found")
                        .Flush();

                    throw std::runtime_error("Non-contiguous filters");
                }

                return output;
            }
        }();

        for (const auto& [block, height, type, count, bytes] : data) {
            const auto params = blockchain::internal::GetFilterParams(type);
            const auto& pGCS =
                filters
                    .emplace_back(
                        block->Bytes(),
                        factory::GCS(
                            api_,
                            params.first,
                            params.second,
                            blockchain::internal::BlockHashToFilterKey(
                                block->Bytes()),
                            count,
                            bytes->Bytes()))
                    .second;

            if (false == bool(pGCS)) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to instantiate ")(
                    DisplayString(chain_))(" cfilter")
                    .Flush();

                return;
            }

            const auto& gcs = *pGCS;
            const auto filterHash = gcs.Hash();
            const auto& cfheader = std::get<1>(headers.emplace_back(
                block, gcs.Header(previous->Bytes()), filterHash->Bytes()));

            if (cfheader->empty()) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": failed to calculate ")(
                    DisplayString(chain_))(" cfheader")
                    .Flush();

                return;
            }

            // TODO verify cfheader against checkpoint
            previous = cfheader;
        }
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        return;
    }

    const auto tip = [&] {
        const auto& [hash, height, type, count, filter] = parsed.second.back();

        return block::Position{height, hash};
    }();
    make_blank<block::Position>::value(api_);
    const auto stored =
        database_.StoreFilters(filterType, headers, filters, tip);

    if (stored) {
        LogNormal(DisplayString(chain_))(
            " cfheader and cfilter chain updated to height ")(tip.first)
            .Flush();
        cb_(filterType, tip);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Database error ").Flush();
    }
}

auto FilterOracle::process_block(
    const filter::Type filterType,
    const block::bitcoin::Block& block) const noexcept
    -> std::unique_ptr<const GCS>
{
    const auto& id = block.ID();
    const auto params = blockchain::internal::GetFilterParams(filterType);
    const auto elements = [&] {
        const auto input = block.ExtractElements(filterType);
        auto output = std::vector<OTData>{};
        std::transform(
            input.begin(),
            input.end(),
            std::back_inserter(output),
            [&](const auto& element) -> OTData {
                return api_.Factory().Data(reader(element));
            });

        return output;
    }();

    return factory::GCS(
        api_,
        params.first,
        params.second,
        blockchain::internal::BlockHashToFilterKey(id.Bytes()),
        elements);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& position,
    const std::optional<bool> resetHeader,
    const std::optional<bool> resetfilter) const noexcept -> bool
{
    return reset_tips_to(
        type,
        database_.FilterHeaderTip(default_type_),
        database_.FilterTip(default_type_),
        position,
        resetHeader,
        resetfilter);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& headerTip,
    const block::Position& position,
    const std::optional<bool> resetHeader) const noexcept -> bool
{
    return reset_tips_to(
        type,
        headerTip,
        database_.FilterTip(default_type_),
        position,
        resetHeader);
}

auto FilterOracle::reset_tips_to(
    const filter::Type type,
    const block::Position& headerTip,
    const block::Position& filterTip,
    const block::Position& position,
    std::optional<bool> resetHeader,
    std::optional<bool> resetfilter) const noexcept -> bool
{
    auto counter{0};

    if (false == resetHeader.has_value()) {
        resetHeader = headerTip > position;
    }

    if (false == resetfilter.has_value()) {
        resetfilter = filterTip > position;
    }

    OT_ASSERT(resetHeader.has_value());
    OT_ASSERT(resetfilter.has_value());

    auto lock = Lock{lock_};
    using Future = std::shared_future<filter::pHeader>;
    auto previous = [&]() -> Future {
        const auto& block = header_.LoadHeader(position.second);

        OT_ASSERT(block);

        auto promise = std::promise<filter::pHeader>{};
        promise.set_value(database_.LoadFilterHeader(
            default_type_, block->ParentHash().Bytes()));

        return promise.get_future();
    }();
    auto resetBlock{false};

    if (resetHeader.value()) {
        if (header_downloader_) {
            header_downloader_->Reset(position, Future{previous});
        }

        resetBlock = true;
        ++counter;
    }

    if (resetfilter.value()) {
        if (filter_downloader_) {
            filter_downloader_->Reset(position, Future{previous});
        }

        resetBlock = true;
        ++counter;
    }

    if (resetBlock && block_indexer_) {
        block_indexer_->Reset(position, std::move(previous));
    }

    return 0 < counter;
}

auto FilterOracle::Shutdown() noexcept -> std::shared_future<void>
{
    init_.get();
    auto lock = Lock{lock_};

    if (header_downloader_) { header_downloader_.reset(); }

    if (filter_downloader_) { filter_downloader_.reset(); }

    if (block_indexer_) { block_indexer_.reset(); }

    try {
        shutdown_promise_.set_value();
    } catch (...) {
    }

    return shutdown_;
}

auto FilterOracle::Start() noexcept -> void
{
    compare_tips_to_header_chain();
    compare_tips_to_checkpoint();
    init_promise_.set_value();
    auto lock = Lock{lock_};

    if (header_downloader_) { header_downloader_->Start(); }

    if (filter_downloader_) { filter_downloader_->Start(); }

    if (block_indexer_) { block_indexer_->Start(); }
}

FilterOracle::~FilterOracle() { Shutdown().get(); }
}  // namespace opentxs::blockchain::client::implementation
