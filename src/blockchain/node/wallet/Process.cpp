// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/node/wallet/Process.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <iterator>
#include <memory>
#include <thread>
#include <type_traits>

#include "blockchain/node/wallet/Accounts.hpp"
#include "blockchain/node/wallet/BlockIndex.hpp"
#include "blockchain/node/wallet/Index.hpp"
#include "blockchain/node/wallet/Progress.hpp"
#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::node::wallet::Process::"

namespace opentxs::blockchain::node::wallet
{
Process::Process(SubchainStateData& parent, Progress& progress) noexcept
    : Job(parent)
    , progress_(progress)
    , cache_(parent_)
    , waiting_()
    , processing_()
{
}

Process::Cache::Cache(const SubchainStateData& parent) noexcept
    : parent_(parent)
    , limit_(params::Data::Chains()
                 .at(parent_.node_.Chain())
                 .block_download_batch_)
    , lock_()
    , pending_()
    , downloading_()
{
}

auto Process::Cache::Pop(BlockMap& dest) noexcept -> bool
{
    if (auto lock = Lock{lock_, std::defer_lock}; lock.try_lock()) {
        auto hashes = std::vector<block::pHash>{};
        move_nodes(
            downloading_,
            dest,
            [&](auto i) {
                using State = std::future_status;
                static constexpr auto zero = std::chrono::microseconds{0};
                const auto& [cookie, data] = *i;
                const auto& [position, future] = data;
                const auto& [height, hash] = position;

                if (auto state = future.wait_for(zero); State::ready == state) {
                    LogVerbose(OT_METHOD)(__func__)(": ")(parent_.name_)(
                        " ready to process block ")(hash->asHex())
                        .Flush();

                    return true;
                } else {
                    LogVerbose(OT_METHOD)(__func__)(": ")(parent_.name_)(
                        " waiting for block ")(hash->asHex())(" to download")
                        .Flush();

                    return false;
                }
            },
            [&](auto move) {
                const auto downloading = dest.size() + move;

                return downloading >= limit_;
            },
            [&](auto it) {
                const auto& [cookie, data] = *it;
                const auto& [position, future] = data;
                const auto& [height, hash] = position;
                hashes.emplace_back(hash);
            });

        if (0u < hashes.size()) { parent_.block_index_.Forget(hashes); }

        download(lock);
        LogInsane(OT_METHOD)(__func__)(": ")(parent_.name_)(
            " staged block count:      ")(pending_.size())
            .Flush();
        LogInsane(OT_METHOD)(__func__)(": ")(parent_.name_)(
            " downloading block count: ")(downloading_.size())
            .Flush();
        LogInsane(OT_METHOD)(__func__)(": ")(parent_.name_)(
            " downloaded block count:  ")(dest.size())
            .Flush();

        return 0u < downloading_.size();
    } else {

        return true;
    }
}

auto Process::Cache::download(const Lock& lock) noexcept -> void
{
    auto positions = std::vector<block::Position>{};
    auto count{downloading_.size()};

    while ((0 < pending_.size()) && (limit_ > count)) {
        ++count;
        auto& position = pending_.front();
        positions.emplace_back(position);
        pending_.pop_front();
    }

    if (0u < positions.size()) { parent_.block_index_.Add(positions); }

    for (auto& position : positions) {
        auto future = parent_.node_.BlockOracle().LoadBitcoin(position.second);
        downloading_.try_emplace(
            next(), std::move(position), std::move(future));
    }
}

auto Process::Cache::Push(std::vector<block::Position>& blocks) noexcept -> void
{
    auto lock = Lock{lock_};
    std::move(blocks.begin(), blocks.end(), std::back_inserter(pending_));
    download(lock);
}

auto Process::Cache::Reorg(const block::Position& parent) noexcept -> void
{
    auto lock = Lock{lock_};

    for (auto i{pending_.begin()}; i != pending_.end();) {
        if (*i > parent) {
            i = pending_.erase(i);
        } else {
            ++i;
        }
    }

    flush(parent, downloading_);
}

auto Process::Do(const Cookie key) noexcept -> void
{
    const auto start = Clock::now();
    const auto& db = parent_.db_;
    const auto& name = parent_.name_;
    const auto& type = parent_.filter_type_;
    const auto& node = parent_.node_;
    const auto& filters = node.FilterOracleInternal();
    const auto& data = [&]() -> auto&
    {
        auto lock = Lock{lock_};

        return processing_.at(key);
    }
    ();
    const auto& [position, future] = data;
    const auto& blockHash = position.second.get();
    auto matchCount = std::size_t{0};
    auto processed{false};
    auto postcondition = ScopeGuard{[&] {
        const auto& processedPosition = data.first;

        if (processed) {
            progress_.UpdateProcess(processedPosition);
            parent_.get_index().Processed(processedPosition, matchCount);
        }

        auto lock = Lock{lock_};
        processing_.erase(key);
        finish(lock);
    }};
    const auto pBlock = future.get();

    if (false == bool(pBlock)) {
        LogVerbose(OT_METHOD)(__func__)(": ")(name)(" invalid block ")(
            blockHash.asHex())
            .Flush();
        Request({position});

        return;
    }

    processed = true;
    const auto& block = *pBlock;
    auto tested = node::internal::WalletDatabase::MatchingIndices{};
    auto [elements, utxos, targets, outpoints] =
        parent_.get_block_targets(blockHash, tested);
    const auto pFilter = filters.LoadFilter(type, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    auto potential = node::internal::WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(targets)) {
        // NOTE GCS::Match returns const_iterators to items in the input vector
        const auto pos = std::distance(targets.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed = block.FindMatches(type, outpoints, potential);
    const auto& [utxo, general] = confirmed;
    matchCount = general.size();
    const auto& oracle = node.HeaderOracleInternal();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;

    OT_ASSERT(position == header.Position());

    parent_.handle_confirmed_matches(block, position, confirmed);
    LogVerbose(OT_METHOD)(__func__)(": ")(name)(" block ")(block.ID().asHex())(
        " at height ")(position.first)(" processed in ")(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start)
            .count())(" milliseconds. ")(matchCount)(" of ")(potential.size())(
        " potential matches confirmed.")
        .Flush();
    db.SubchainMatchBlock(parent_.db_key_, tested, blockHash.Bytes());

    if (false == parent_.parent_.Query()) {
        // NOTE reduce lock and thread pool contention during initial sync
        static constexpr auto rateLimit = std::chrono::seconds{1};
        static constexpr auto zero = std::chrono::milliseconds{0};
        const auto elapsed =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - start);
        Sleep(std::max(rateLimit - elapsed, zero));
    }
}

auto Process::flush(const block::Position& parent, BlockMap& map) noexcept
    -> void
{
    for (auto i{map.begin()}; i != map.end();) {
        const auto& [id, data] = *i;
        const auto& [position, future] = data;

        if (position > parent) {
            i = map.erase(i);
        } else {
            ++i;
        }
    }
}

auto Process::limit(const Lock& lock, const std::size_t outstanding)
    const noexcept -> bool
{
    const auto running = processing_.size() + outstanding;
    const auto limit = [&]() -> std::size_t {
        const auto cores = std::max<std::size_t>(
            std::max(std::thread::hardware_concurrency(), 1u) - 1u, 1u);

        if (parent_.parent_.Query()) {

            return cores;
        } else {

            return 1u;
        }
    }();

    return running >= limit;
}

auto Process::move_nodes(
    BlockMap& from,
    BlockMap& to,
    std::function<bool(BlockMap::iterator)> moveCondition,
    std::function<bool(std::size_t)> breakCondition,
    std::function<void(BlockMap::iterator)> post) noexcept -> void
{
    OT_ASSERT(moveCondition);
    OT_ASSERT(breakCondition);

    auto move = std::vector<BlockMap::iterator>{};

    for (auto i{from.begin()}; i != from.end(); ++i) {
        if (breakCondition(move.size())) { break; }

        if (moveCondition(i)) { move.emplace_back(i); }
    }

    move_nodes(move, from, to, post);
}

auto Process::move_nodes(
    std::vector<BlockMap::iterator>& items,
    BlockMap& from,
    BlockMap& to,
    std::function<void(BlockMap::iterator)> post) noexcept -> void
{
    for (auto& i : items) {
        const auto insert = to.insert(from.extract(i));

        OT_ASSERT(insert.inserted);

        if (post) { post(insert.position); }
    }

    items.clear();
}

auto Process::next() noexcept -> Cookie
{
    static auto counter = std::atomic<Cookie>{-1};

    return ++counter;
}

auto Process::Reorg(const block::Position& parent) noexcept -> void
{
    cache_.Reorg(parent);
    flush(parent, waiting_);
    flush(parent, processing_);
}

auto Process::Request(std::vector<block::Position> blocks) noexcept -> void
{
    cache_.Push(blocks);
}

auto Process::Run() noexcept -> bool
{
    auto again{false};
    auto lock = Lock{lock_};
    again |= cache_.Pop(waiting_);
    move_nodes(
        waiting_,
        processing_,
        [](auto i) { return true; },
        [&](auto move) { return limit(lock, move); },
        [this](auto i) {
            const auto& cookie = i->first;
            static constexpr auto job{"process"};
            queue_work([=] { Do(cookie); }, job, true);
        });
    again |= (0u < waiting_.size());
    again |= (0u < processing_.size());

    return again;
}
}  // namespace opentxs::blockchain::node::wallet
