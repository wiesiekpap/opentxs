// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <queue>
#include <string>

#include "blockchain/node/wallet/Scan.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace node
{
namespace wallet
{
class Process;
class Progress;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Rescan final : public Scan
{
public:
    auto Queue(const block::Position& position) noexcept -> void;
    auto Reorg(const block::Position& parent) noexcept -> void final;

    Rescan(
        SubchainStateData& parent,
        Process& process,
        Progress& progress,
        Scan& scan) noexcept;

    ~Rescan() final = default;

private:
    Scan& scan_;
    mutable std::mutex rescan_lock_;
    std::queue<block::Position> rescan_requests_;

    auto get_stop() const noexcept -> block::Height final;
    auto type() const noexcept -> const char* final { return "rescan"; }

    auto caught_up() noexcept -> void final;
    auto flush(const Lock& lock) noexcept -> void final;
    auto handle_stopped() noexcept -> void final;
    auto process(const Lock& lock, const block::Position& position) noexcept
        -> void;

    Rescan() = delete;
    Rescan(const Rescan&) = delete;
    Rescan(Rescan&&) = delete;
    auto operator=(const Rescan&) -> Rescan& = delete;
    auto operator=(Rescan&&) -> Rescan& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
