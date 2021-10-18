// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/node/wallet/Progress.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::node::wallet::Progress::Imp::"

namespace opentxs::blockchain::node::wallet
{
struct Progress::Imp {
    auto Dirty() const noexcept -> std::optional<block::Position>
    {
        auto lock = Lock{lock_};

        return lowest_dirty(lock);
    }
    auto Get() const noexcept -> block::Position
    {
        auto lock = Lock{lock_};

        return last_reported_.value_or(parent_.null_position_);
    }

    auto Reorg(const block::Position& parent) noexcept -> void
    {
        auto lock = Lock{lock_};
        last_reported_ = std::nullopt;

        if (highest_clean_.has_value() && highest_clean_.value() > parent) {
            highest_clean_ = parent;
        }

        for (auto i = dirty_blocks_.begin(); i != dirty_blocks_.end();) {
            const auto& position = *i;

            if (position > parent) {
                i = dirty_blocks_.erase(i);
            } else {
                ++i;
            }
        }

        report(lock);
    }
    auto UpdateProcess(const ProgressBatch& data) noexcept -> void
    {
        auto lock = Lock{lock_};

        for (const auto& [position, matches] : data) {
            dirty_blocks_.erase(position);
        }

        report(lock);
    }
    auto UpdateScan(
        const std::optional<block::Position>& highestClean,
        const std::vector<block::Position>& in) noexcept -> void
    {
        auto lock = Lock{lock_};
        std::copy(
            in.begin(),
            in.end(),
            std::inserter(dirty_blocks_, dirty_blocks_.end()));

        if (highestClean.has_value()) {
            set_highest_clean(lock, highestClean.value());
        }

        report(lock);
    }

    Imp(const SubchainStateData& parent) noexcept
        : parent_(parent)
        , lock_()
        , last_reported_(std::nullopt)
        , highest_clean_(std::nullopt)
        , dirty_blocks_()
    {
    }

    ~Imp() = default;

private:
    using Map = std::map<block::Position, long long int>;

    const SubchainStateData& parent_;
    mutable std::mutex lock_;
    std::optional<block::Position> last_reported_;
    std::optional<block::Position> highest_clean_;
    std::set<block::Position> dirty_blocks_;

    auto lowest_dirty(const Lock&) const noexcept
        -> std::optional<block::Position>
    {
        if (dirty_blocks_.empty()) { return std::nullopt; }

        return *dirty_blocks_.begin();
    }

    auto report(const Lock& lock) noexcept -> void
    {
        const auto& best = highest_clean_.value_or(parent_.null_position_);
        const auto report = [&] {
            if (last_reported_.has_value()) {
                const auto& value = last_reported_.value();

                return value != best;
            } else {

                return true;
            }
        }();

        if (report) {
            LogVerbose(OT_METHOD)(__func__)(": ")(parent_.name_)(" progress: ")(
                best.first)
                .Flush();
            parent_.update_scan(best);
            last_reported_ = std::move(best);
        }
    }
    auto set_highest_clean(
        const Lock& lock,
        const block::Position& pos) noexcept -> void
    {
        const bool wantHighestClean = [&] {
            if (highest_clean_.has_value()) {

                return pos > highest_clean_.value();
            } else {

                return true;
            }
        }();
        const bool setHighestClean = [&] {
            if (wantHighestClean) {
                const auto dirty = lowest_dirty(lock);

                if (dirty.has_value()) {

                    return dirty.value() > pos;
                } else {

                    return true;
                }
            } else {

                return false;
            }
        }();

        if (setHighestClean) { highest_clean_ = pos; }
    }

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

Progress::Progress(const SubchainStateData& parent) noexcept
    : imp_(std::make_unique<Imp>(parent).release())
{
}

auto Progress::Dirty() const noexcept -> std::optional<block::Position>
{
    return imp_->Dirty();
}

auto Progress::Get() const noexcept -> block::Position { return imp_->Get(); }

auto Progress::Reorg(const block::Position& parent) noexcept -> void
{
    imp_->Reorg(parent);
}

auto Progress::UpdateProcess(const ProgressBatch& processed) noexcept -> void
{
    imp_->UpdateProcess(processed);
}

auto Progress::UpdateScan(
    const std::optional<block::Position>& highestClean,
    const std::vector<block::Position>& dirtyBlocks) noexcept -> void
{
    imp_->UpdateScan(highestClean, dirtyBlocks);
}

Progress::~Progress()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::blockchain::node::wallet
