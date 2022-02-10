// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <memory>

#include "opentxs/blockchain/Blockchain.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

class Identifier;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Actor
{
public:
    virtual auto FinishBackgroundTasks() noexcept -> void = 0;
    virtual auto ProcessBlockAvailable(const block::Hash& block) noexcept
        -> void = 0;
    virtual auto ProcessKey() noexcept -> void = 0;
    virtual auto ProcessMempool(
        std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void = 0;
    virtual auto ProcessNewFilter(const block::Position& tip) noexcept
        -> void = 0;
    virtual auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& ancestor) noexcept -> bool = 0;
    virtual auto ProcessStateMachine(bool enabled) noexcept -> bool = 0;
    virtual auto ProcessTaskComplete(
        const Identifier& id,
        const char* type,
        bool enabled) noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    virtual ~Actor() = default;

protected:
    Actor() = default;

private:
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    Actor& operator=(const Actor&) = delete;
    Actor& operator=(Actor&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
