// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/node/wallet/Scan.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/Accounts.hpp"
#include "blockchain/node/wallet/Process.hpp"
#include "blockchain/node/wallet/Progress.hpp"
#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"  // IWYU pragma: keep
#include "util/ScopeGuard.hpp"

#define OT_METHOD "opentxs::blockchain::node::wallet::Scan::"

namespace opentxs::blockchain::node::wallet
{
Scan::Scan(
    SubchainStateData& parent,
    Process& process,
    Progress& progress) noexcept
    : Job(parent)
    , progress_(progress)
    , last_scanned_(parent_.db_.SubchainLastScanned(parent_.db_key_))
    , ready_(false)
    , process_(process)
    , filter_tip_(std::nullopt)
{
}

auto Scan::caught_up() noexcept -> void
{
    parent_.parent_.Complete(parent_.db_key_);
}

auto Scan::Do(
    const block::Position best,
    const block::Height stop,
    block::Position highestTested) noexcept -> void
{
    const auto& api = parent_.api_;
    const auto& name = parent_.name_;
    const auto& node = parent_.node_;
    const auto& type = parent_.filter_type_;
    const auto& headers = node.HeaderOracleInternal();
    const auto& filters = node.FilterOracleInternal();
    auto atLeastOnce{false};
    auto highestClean = std::optional<block::Position>{std::nullopt};
    auto dirty = std::vector<block::Position>{};
    auto postcondition = ScopeGuard{[&] {
        auto lock = Lock{lock_};

        if (atLeastOnce) {
            progress_.UpdateScan(highestClean, std::move(dirty));
            last_scanned_ = highestTested;
        }

        finish(lock);
    }};

    const auto start = Clock::now();
    const auto startHeight = highestTested.first + 1;
    const auto stopHeight =
        std::min(std::min(startHeight + 9999, best.first), stop);

    if (startHeight > stopHeight) { return; }

    LogVerbose(OT_METHOD)(__func__)(": ")(name)(" ")(this->type())(
        "ning filters from ")(startHeight)(" to ")(stopHeight)
        .Flush();
    const auto [elements, utxos, patterns] = parent_.get_account_targets();
    auto blockHash = api.Factory().Data();
    auto cache = std::vector<block::Position>{};

    for (auto i{startHeight}; i <= stopHeight; ++i) {
        if (shutdown_) { return; }

        blockHash = headers.BestHash(i, best);

        if (blockHash->empty()) {
            LogVerbose(OT_METHOD)(__func__)(": ")(name)(" interrupting ")(
                this->type())(" due to chain reorg")
                .Flush();

            break;
        }

        auto testPosition = block::Position{i, blockHash};
        const auto pFilter = filters.LoadFilterOrResetTip(type, testPosition);

        if (false == bool(pFilter)) {
            LogVerbose(OT_METHOD)(__func__)(": ")(name)(" filter at height ")(
                i)(" not found ")
                .Flush();

            break;
        }

        atLeastOnce = true;
        const auto& filter = *pFilter;
        auto matches = filter.Match(patterns);
        auto isClean{true};

        if (0 < matches.size()) {
            const auto [untested, retest] =
                parent_.get_block_targets(blockHash, utxos);
            matches = filter.Match(retest);

            if (0 < matches.size()) {
                LogVerbose(OT_METHOD)(__func__)(": ")(name)(" GCS ")(
                    this->type())(" for block ")(blockHash->asHex())(
                    " at height ")(i)(" found ")(matches.size())(
                    " new potential matches for the ")(patterns.size())(
                    " target elements for ")(parent_.id_)
                    .Flush();
                cache.emplace_back(testPosition);
                isClean = false;
            }
        }

        if (isClean) {
            if (0u == dirty.size()) { highestClean = testPosition; }
        } else {
            dirty.emplace_back(testPosition);
        }

        highestTested = std::move(testPosition);
    }

    if (atLeastOnce) {
        const auto count = cache.size();
        LogVerbose(OT_METHOD)(__func__)(": ")(name)(" ")(this->type())(
            " found ")(count)(" new potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                Clock::now() - start)
                .count())(" milliseconds")
            .Flush();
        process_.Request(std::move(cache));
    } else {
        LogVerbose(OT_METHOD)(__func__)(": ")(name)(" ")(this->type())(
            " interrupted due to missing filter")
            .Flush();
    }
}

auto Scan::flush(const Lock&) noexcept -> void {}

auto Scan::get_stop() const noexcept -> block::Height
{
    return std::numeric_limits<block::Height>::max();
}

auto Scan::handle_stopped() noexcept -> void
{
    const auto& name = parent_.name_;
    LogVerbose(OT_METHOD)(__func__)(": ")(name)(" ")(this->type())(
        " progress paused")
        .Flush();
}

auto Scan::Position() const noexcept -> block::Position
{
    auto lock = Lock{lock_};

    return last_scanned_.value_or(parent_.null_position_);
}

auto Scan::Reorg(const block::Position& parent) noexcept -> void
{
    if (last_scanned_.has_value() && (last_scanned_.value() > parent)) {
        last_scanned_ = parent;
    }

    if (filter_tip_.has_value() && (filter_tip_.value() > parent)) {
        filter_tip_ = parent;
    }
}

auto Scan::Run() noexcept -> bool
{
    const auto tip = [this] {
        auto lock = Lock{lock_};

        if (false == filter_tip_.has_value()) {
            const auto& node = parent_.node_;
            const auto& filters = node.FilterOracleInternal();
            filter_tip_ = filters.FilterTip(parent_.filter_type_);
        }

        OT_ASSERT(filter_tip_.has_value());

        return filter_tip_.value();
    }();

    return Run(tip);
}

auto Scan::Run(const block::Position& filterTip) noexcept -> bool
{
    const auto& name = parent_.name_;
    const auto& null = parent_.null_position_;
    auto needScan{false};

    try {
        const auto current = [&] {
            auto lock = Lock{lock_};
            filter_tip_ = filterTip;
            flush(lock);

            if (is_running(lock)) {
                throw std::runtime_error{"job is already running"};
            }

            if (false == ready_) {
                throw std::runtime_error{"job is disabled"};
            }

            return last_scanned_;
        }();
        const auto stop = get_stop();
        // NOTE below this line we can assume only one Scan thread is running
        if (current.has_value()) {
            if (current == filterTip) {
                LogVerbose(OT_METHOD)(__func__)(": ")(name)(" has been ")(
                    type())("ned to the newest downloaded cfilter ")(
                    filterTip.second->asHex())(" at height ")(filterTip.first)
                    .Flush();
                needScan = false;
            } else if (current > filterTip) {
                {
                    auto lock = Lock{lock_};
                    last_scanned_ = parent_.node_.HeaderOracle().GetPosition(
                        std::max<block::Height>(filterTip.first, 0) - 1);
                }
                needScan = true;
            } else {
                needScan = true;
            }
        } else {
            needScan = true;
        }

        if (needScan) {
            auto highestTested = current.value_or(null);

            if (highestTested.first > stop) {
                handle_stopped();
            } else {
                needScan = queue_work(
                    [=,
                     last = std::move(highestTested),
                     best = std::move(filterTip)] {
                        Do(std::move(best), stop, std::move(last));
                    },
                    type(),
                    false);
            }
        } else {
            progress_.UpdateScan(filterTip, {});
            caught_up();
        }
    } catch (...) {
        needScan = true;
    }

    return needScan;
}

auto Scan::SetReady() noexcept -> void
{
    auto lock = Lock{lock_};
    set_ready(lock);
}

auto Scan::set_ready(const Lock& lock) noexcept -> void { ready_ = true; }
}  // namespace opentxs::blockchain::node::wallet
