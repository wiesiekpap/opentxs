// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/client/wallet/SubchainStateData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <future>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "util/JobCounter.hpp"
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::client::wallet::SubchainStateData::"

namespace opentxs::blockchain::client::internal
{
auto Wallet::ProcessThreadPool(const zmq::Message& in) noexcept -> void
{
    const auto body = in.Body();

    if (2 > body.size()) {
        LogOutput("opentxs::blockchain::client::internal:Wallet::")(
            __FUNCTION__)(": Invalid message")
            .Flush();

        OT_FAIL;
    }

    auto* pData = reinterpret_cast<client::wallet::SubchainStateData*>(
        body.at(1).as<std::uintptr_t>());

    OT_ASSERT(nullptr != pData);

    auto& data = *pData;
    auto postcondition = ScopeGuard{[&] {
        data.running_.store(false);
        data.task_finished_();
        --data.job_counter_;
    }};

    switch (body.at(0).as<Task>()) {
        case Task::index: {
            data.index();
        } break;
        case Task::scan: {
            data.scan();
        } break;
        case Task::process: {
            data.process();
        } break;
        case Task::reorg: {
            data.reorg();
        } break;
        default: {
            OT_FAIL;
        }
    }
}
}  // namespace opentxs::blockchain::client::internal

namespace opentxs::blockchain::client::wallet
{
SubchainStateData::SubchainStateData(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const internal::Network& network,
    const WalletDatabase& db,
    const OTIdentifier&& id,
    const SimpleCallback& taskFinished,
    Outstanding& jobCounter,
    const zmq::socket::Push& threadPool,
    const filter::Type filter,
    const Subchain subchain) noexcept
    : id_(std::move(id))
    , subchain_(subchain)
    , job_counter_(jobCounter)
    , task_finished_(taskFinished)
    , running_(false)
    , reorg_()
    , last_indexed_()
    , last_scanned_()
    , blocks_to_request_()
    , outstanding_blocks_()
    , process_block_queue_()
    , api_(api)
    , blockchain_(blockchain)
    , network_(network)
    , db_(db)
    , filter_type_(filter)
    , thread_pool_(threadPool)
{
    OT_ASSERT(task_finished_);
    OT_ASSERT(false == id_->empty());
}

auto SubchainStateData::ReorgQueue::Empty() const noexcept -> bool
{
    Lock lock(lock_);

    return 0 == parents_.size();
}

auto SubchainStateData::ReorgQueue::Queue(
    const block::Position& parent) noexcept -> bool
{
    Lock lock(lock_);
    parents_.push(parent);

    return true;
}

auto SubchainStateData::ReorgQueue::Next() noexcept -> block::Position
{
    OT_ASSERT(false == Empty());

    Lock lock(lock_);
    auto output{parents_.front()};
    parents_.pop();

    return output;
}

auto SubchainStateData::check_blocks() noexcept -> bool
{
    for (const auto& hash : blocks_to_request_) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" requesting block ")(
            hash->asHex())(" queue position: ")(outstanding_blocks_.size())
            .Flush();

        if (0 == outstanding_blocks_.count(hash)) {
            auto [it, added] = outstanding_blocks_.emplace(
                hash, network_.BlockOracle().LoadBitcoin(hash));

            OT_ASSERT(added);

            process_block_queue_.push(it);
        }
    }

    blocks_to_request_.clear();

    return false;
}

auto SubchainStateData::check_process() noexcept -> bool
{
    if (process_block_queue_.empty()) { return false; }

    const auto& [id, future] = *process_block_queue_.front();

    if (std::future_status::ready ==
        future.wait_for(std::chrono::milliseconds(1))) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
            " ready to process block")
            .Flush();
        static constexpr auto job{"process"};

        return queue_work(Task::process, job);
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" waiting for block ")(
            id->asHex())(" to download")
            .Flush();
    }

    return false;
}

auto SubchainStateData::check_reorg() noexcept -> bool
{
    if (reorg_.Empty()) { return false; }

    static constexpr auto job{"reorg"};

    return queue_work(Task::reorg, job);
}

auto SubchainStateData::check_scan() noexcept -> bool
{
    auto needScan{false};

    if (last_scanned_.has_value()) {
        const auto bestFilter =
            network_.FilterOracleInternal().FilterTip(filter_type_);

        if (last_scanned_ == bestFilter) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
                " has been scanned to the newest downloaded "
                "filter ")(bestFilter.second->asHex())(" at height ")(
                bestFilter.first)
                .Flush();
        } else {
            const auto [ancestor, best] =
                network_.HeaderOracleInternal().CommonParent(
                    last_scanned_.value());
            last_scanned_ = ancestor;

            if (last_scanned_ == best) {
                LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
                    " has been scanned to current best block ")(
                    best.second->asHex())(" at height ")(best.first)
                    .Flush();
            } else {
                needScan = true;
                LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
                    " scanning progress: ")(last_scanned_.value().first)
                    .Flush();
            }
        }
    } else {
        needScan = true;
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" scanning progress: ")(
            0)
            .Flush();
    }

    if (needScan) {
        static constexpr auto job{"scan"};

        return queue_work(Task::scan, job);
    }

    return false;
}

auto SubchainStateData::get_targets(
    const internal::WalletDatabase::Patterns& keys,
    const std::vector<internal::WalletDatabase::UTXO>& unspent) const noexcept
    -> client::GCS::Targets
{
    auto output = client::GCS::Targets{};
    output.reserve(keys.size() + unspent.size());
    std::transform(
        std::begin(keys),
        std::end(keys),
        std::back_inserter(output),
        [](const auto& in) { return reader(in.second); });

    switch (filter_type_) {
        case filter::Type::Basic_BIP158: {
            std::transform(
                std::begin(unspent),
                std::end(unspent),
                std::back_inserter(output),
                [](const auto& in) {
                    return ReadView{
                        in.second.script().data(), in.second.script().size()};
                });
        } break;
        case filter::Type::Basic_BCHVariant:
        case filter::Type::Extended_opentxs:
        default: {
            std::transform(
                std::begin(unspent),
                std::end(unspent),
                std::back_inserter(output),
                [](const auto& in) { return in.first.Bytes(); });
        }
    }

    return output;
}

auto SubchainStateData::index_element(
    const filter::Type type,
    const api::client::blockchain::BalanceNode::Element& input,
    const Bip32Index index,
    WalletDatabase::ElementMap& output) noexcept -> void
{
    const auto pubkeyHash = input.PubkeyHash();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
        " indexing public key with hash ")(pubkeyHash->asHex())
        .Flush();
    auto& list = output[index];
    auto scripts = std::vector<std::unique_ptr<const block::bitcoin::Script>>{};
    scripts.reserve(4);  // WARNING keep this number up to date if new scripts
                         // are added
    const auto& p2pk = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2PK(network_.Chain(), *input.Key()));
    const auto& p2pkh = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2PKH(network_.Chain(), *input.Key()));
    const auto& p2sh_p2pk = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2SH(network_.Chain(), *p2pk));
    const auto& p2sh_p2pkh = scripts.emplace_back(
        api_.Factory().BitcoinScriptP2SH(network_.Chain(), *p2pkh));

    OT_ASSERT(p2pk);
    OT_ASSERT(p2pkh);
    OT_ASSERT(p2sh_p2pk);
    OT_ASSERT(p2sh_p2pkh);

    switch (type) {
        case filter::Type::Extended_opentxs: {
            OT_ASSERT(p2pk->Pubkey().has_value());
            OT_ASSERT(p2pkh->PubkeyHash().has_value());
            OT_ASSERT(p2sh_p2pk->ScriptHash().has_value());
            OT_ASSERT(p2sh_p2pkh->ScriptHash().has_value());

            list.emplace_back(space(p2pk->Pubkey().value()));
            list.emplace_back(space(p2pkh->PubkeyHash().value()));
            list.emplace_back(space(p2sh_p2pk->ScriptHash().value()));
            list.emplace_back(space(p2sh_p2pkh->ScriptHash().value()));
        } break;
        case filter::Type::Basic_BIP158:
        case filter::Type::Basic_BCHVariant:
        default: {
            for (const auto& script : scripts) {
                script->Serialize(writer(list.emplace_back()));
            }
        }
    }
}

auto SubchainStateData::process() noexcept -> void
{
    const auto start = Clock::now();
    const auto& filters = network_.FilterOracleInternal();
    auto it = process_block_queue_.front();
    auto postcondition = ScopeGuard{[&] {
        outstanding_blocks_.erase(it);
        process_block_queue_.pop();
    }};
    const auto& blockHash = it->first.get();
    const auto pBlock = it->second.get();

    if (false == bool(pBlock)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" invalid block ")(
            blockHash.asHex())
            .Flush();
        auto& vector = blocks_to_request_;
        vector.emplace(vector.begin(), blockHash);

        return;
    }

    const auto& block = *pBlock;
    auto tested = WalletDatabase::MatchingIndices{};
    auto patterns = client::GCS::Targets{};
    auto elements = db_.GetUntestedPatterns(
        id_, subchain_, filter_type_, blockHash.Bytes());
    std::for_each(
        std::begin(elements), std::end(elements), [&](const auto& pattern) {
            const auto& [id, data] = pattern;
            const auto& [index, subchain] = id;
            tested.emplace_back(index);
            patterns.emplace_back(reader(data));
        });

    const auto pFilter = filters.LoadFilter(filter_type_, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    auto potential = WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(patterns)) {
        const auto pos = std::distance(patterns.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed =
        block.FindMatches(blockchain_, filter_type_, {}, potential);
    const auto& oracle = network_.HeaderOracleInternal();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;
    handle_confirmed_matches(block, header.Position(), confirmed);
    const auto [balance, unconfirmed] = db_.GetBalance();
    LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" block ")(
        block.ID().asHex())(" processed in ")(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start)
            .count())(" milliseconds. ")(confirmed.size())(" of ")(
        potential.size())(" potential matches confirmed. Wallet balance is: ")(
        unconfirmed)(" (")(balance)(" confirmed)")
        .Flush();
    db_.SubchainMatchBlock(
        id_, subchain_, filter_type_, tested, blockHash.Bytes());

    if (0 < confirmed.size()) {
        // Re-scan the last 1000 blocks
        const auto height = std::max(header.Height() - 1000, block::Height{0});
        last_scanned_ = block::Position{height, oracle.BestHash(height)};
    }
}

auto SubchainStateData::queue_work(const Task task, const char* log) noexcept
    -> bool
{
    constexpr auto limit = std::chrono::minutes{1};
    const auto start = Clock::now();

    while (job_counter_.limited()) {
        Sleep(std::chrono::microseconds(100));

        if ((Clock::now() - start) > limit) { OT_FAIL; }
    }

    using Pool = internal::ThreadPool;

    auto work = Pool::MakeWork(api_, network_.Chain(), Pool::Work::HDAccount);
    work->AddFrame(task);
    work->AddFrame(reinterpret_cast<std::uintptr_t>(this));
    running_.store(true);

    if (thread_pool_.Send(work)) {
        LogDebug(OT_METHOD)(__FUNCTION__)(": ")(id_)(" ")(log)(" job queued")
            .Flush();
        ++job_counter_;
    } else {
        LogDebug(OT_METHOD)(__FUNCTION__)(": ")(id_)(" failed to queue ")(log)(
            " job")
            .Flush();

        running_.store(false);
    }

    return running_.load();
}

auto SubchainStateData::reorg() noexcept -> void
{
    while (false == reorg_.Empty()) {
        reorg_.Next();
        const auto tip =
            db_.SubchainLastProcessed(id_, subchain_, filter_type_);
        const auto reorg = network_.HeaderOracleInternal().CalculateReorg(tip);
        db_.ReorgTo(id_, subchain_, filter_type_, reorg);
    }

    const auto scannedTarget =
        network_.HeaderOracleInternal()
            .CommonParent(db_.SubchainLastScanned(id_, subchain_, filter_type_))
            .first;

    if (last_scanned_.has_value()) { last_scanned_ = scannedTarget; }

    blocks_to_request_.clear();
    outstanding_blocks_.clear();

    while (false == process_block_queue_.empty()) {
        process_block_queue_.pop();
    }
}

auto SubchainStateData::scan() noexcept -> void
{
    const auto start = Clock::now();
    const auto& headers = network_.HeaderOracleInternal();
    const auto& filters = network_.FilterOracleInternal();
    const auto best = headers.BestChain();
    const auto startHeight = std::min(
        best.first,
        last_scanned_.has_value() ? last_scanned_.value().first + 1 : 0);
    const auto first =
        last_scanned_.has_value()
            ? block::Position{startHeight, headers.BestHash(startHeight)}
            : block::Position{1, headers.BestHash(1)};
    const auto stopHeight = std::min(
        std::min(startHeight + 9999, best.first),
        filters.FilterTip(filter_type_).first);

    if (first.second->empty()) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
            " resetting due to reorg")
            .Flush();

        return;
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
            " scanning filters from ")(startHeight)(" to ")(stopHeight)
            .Flush();
    }

    const auto elements = db_.GetPatterns(id_, subchain_, filter_type_);
    const auto utxos = db_.GetUnspentOutputs();
    auto highestTested =
        last_scanned_.value_or(make_blank<block::Position>::value(api_));
    auto atLeastOnce{false};

    for (auto i{startHeight}; i <= stopHeight; ++i) {
        auto blockHash = headers.BestHash(i);
        const auto pFilter = filters.LoadFilterOrResetTip(
            filter_type_, block::Position{i, blockHash});

        if (false == bool(pFilter)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": ")(id_)(" filter at height ")(
                i)(" not found ")
                .Flush();

            break;
        }

        atLeastOnce = true;
        highestTested.first = i;
        highestTested.second = blockHash;
        const auto& filter = *pFilter;
        auto patterns = get_targets(elements, utxos);
        auto matches = filter.Match(patterns);
        const auto size{matches.size()};

        if (0 < matches.size()) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" GCS for block ")(
                blockHash->asHex())(" at height ")(i)(
                " matches at least one of the ")(patterns.size())(
                " target elements for ")(id_)
                .Flush();
            const auto retest = db_.GetUntestedPatterns(
                id_, subchain_, filter_type_, blockHash->Bytes());
            patterns = get_targets(retest, utxos);
            matches = filter.Match(patterns);
            LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" ")(matches.size())(
                " of ")(size)(" matches are new")
                .Flush();

            if (0 < matches.size()) {
                blocks_to_request_.emplace_back(std::move(blockHash));
            }
        }
    }

    if (atLeastOnce) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(" found ")(
            blocks_to_request_.size())(" potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - start)
                .count())(" milliseconds")
            .Flush();
        last_scanned_ = std::move(highestTested);
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": ")(id_)(
            " scan interrupted due to missing filter")
            .Flush();
    }
}
auto SubchainStateData::state_machine() noexcept -> bool
{
    if (running_) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": ")(id_)(" task is running")
            .Flush();

        return false;
    }

    if (check_reorg()) { return false; }
    if (check_blocks()) { return false; }
    if (check_index()) { return false; }
    if (check_scan()) { return false; }
    if (check_process()) { return false; }

    return false;
}
}  // namespace opentxs::blockchain::client::wallet
