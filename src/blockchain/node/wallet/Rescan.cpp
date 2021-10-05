// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "blockchain/node/wallet/Rescan.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <limits>
#include <optional>
#include <utility>

#include "blockchain/node/wallet/Progress.hpp"
#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::node::wallet::Rescan::"

namespace opentxs::blockchain::node::wallet
{
Rescan::Rescan(
    SubchainStateData& parent,
    Process& process,
    Progress& progress,
    Scan& scan) noexcept
    : Scan(parent, process, progress)
    , scan_(scan)
    , rescan_lock_()
    , rescan_requests_()
{
}

auto Rescan::caught_up() noexcept -> void
{
    auto lock = Lock{lock_};
    ready_ = false;
    last_scanned_ = std::nullopt;
}

auto Rescan::flush(const Lock& lock) noexcept -> void
{
    auto qLock = Lock{rescan_lock_};
    const auto have = 0 < rescan_requests_.size();

    while (0 < rescan_requests_.size()) {
        {
            auto& position = rescan_requests_.front();
            process(lock, position);
        }

        rescan_requests_.pop();
    }

    if (have) {
        OT_ASSERT(last_scanned_.has_value());

        LogVerbose(OT_METHOD)(__func__)(": ")(parent_.name_)(
            " rescanning from block ")(last_scanned_.value().first)
            .Flush();
    }
}

auto Rescan::get_stop() const noexcept -> block::Height
{
    const auto dirty = [this] {
        auto out = progress_.Dirty();

        if (out.has_value()) {

            return std::max<block::Height>(out.value().first, 1u) - 1u;
        } else {

            return std::numeric_limits<block::Height>::max();
        }
    }();

    return std::min(scan_.Position().first, dirty);
}

auto Rescan::handle_stopped() noexcept -> void
{
    Scan::handle_stopped();
    queue_work([] { Sleep(std::chrono::milliseconds{250}); }, type(), false);
}

auto Rescan::process(const Lock& lock, const block::Position& position) noexcept
    -> void
{
    set_ready(lock);
    LogVerbose(OT_METHOD)(__func__)(": ")(parent_.name_)(
        " incoming position: ")(position.second->asHex())(" at height ")(
        position.first)
        .Flush();
    auto& rescan = last_scanned_;

    if (rescan.has_value() && (rescan.value() != parent_.null_position_)) {
        const auto current = rescan.value();
        LogVerbose(OT_METHOD)(__func__)(":  ")(parent_.name_)(
            " current position: ")(current.second->asHex())(" at height ")(
            current.first)
            .Flush();

        if (current > position) { rescan = position; }
    } else {
        rescan = position;
    }
}

auto Rescan::Reorg(const block::Position& parent) noexcept -> void
{
    {
        auto lock = Lock{lock_};
        flush(lock);
    }

    Scan::Reorg(parent);
}

auto Rescan::Queue(const block::Position& position) noexcept -> void
{
    auto lock = Lock{rescan_lock_};
    rescan_requests_.push(position);
}
}  // namespace opentxs::blockchain::node::wallet
