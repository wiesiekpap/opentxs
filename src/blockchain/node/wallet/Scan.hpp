// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string>

#include "blockchain/node/wallet/Job.hpp"
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
class Scan : public Job
{
public:
    auto Position() const noexcept -> block::Position;

    auto Reorg(const block::Position& parent) noexcept -> void override;
    using Job::Run;
    auto Run() noexcept -> bool final;
    auto Run(const block::Position& filterTip) noexcept -> bool;
    auto SetReady() noexcept -> void;

    Scan(
        SubchainStateData& parent,
        Process& process,
        Progress& progress) noexcept;

    ~Scan() override = default;

protected:
    Progress& progress_;
    std::optional<block::Position> last_scanned_;
    bool ready_;

    virtual auto caught_up() noexcept -> void;
    virtual auto handle_stopped() noexcept -> void;
    auto set_ready(const Lock& lock) noexcept -> void;

private:
    Process& process_;
    std::optional<block::Position> filter_tip_;

    virtual auto get_stop() const noexcept -> block::Height;
    auto type() const noexcept -> const char* override { return "scan"; }

    auto Do(
        const block::Position best,
        const block::Height stop,
        block::Position highestTested) noexcept -> void;
    virtual auto flush(const Lock& lock) noexcept -> void;

    Scan() = delete;
    Scan(const Scan&) = delete;
    Scan(Scan&&) = delete;
    auto operator=(const Scan&) -> Scan& = delete;
    auto operator=(Scan&&) -> Scan& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
