// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/client/Wallet.hpp"  // IWYU pragma: associated

#include <map>
#include <set>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"

// #define OT_METHOD
// "opentxs::blockchain::client::implementation::Wallet::Accounts::"

namespace opentxs::blockchain::client::implementation
{
Wallet::Accounts::Accounts(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const zmq::socket::Push& socket,
    const Type chain,
    const SimpleCallback& taskFinished) noexcept
    : api_(api)
    , blockchain_api_(blockchain)
    , network_(network)
    , db_(db)
    , thread_pool_(socket)
    , task_finished_(taskFinished)
    , chain_(chain)
    , job_counter_()
    , map_(init(
          api,
          blockchain,
          network,
          db,
          socket,
          chain,
          job_counter_,
          taskFinished))
{
}

auto Wallet::Accounts::Add(const identifier::Nym& nym) noexcept -> bool
{
    auto [it, added] = map_.try_emplace(
        nym,
        api_,
        blockchain_api_,
        blockchain_api_.BalanceTree(nym, chain_),
        network_,
        db_,
        thread_pool_,
        job_counter_.Allocate(),
        task_finished_);

    if (added) {
        LogNormal("Initializing ")(DisplayString(chain_))(" wallet for ")(nym)
            .Flush();
    }

    return added;
}

auto Wallet::Accounts::Add(const zmq::Frame& message) noexcept -> bool
{
    auto id = api_.Factory().NymID();
    id->Assign(message.Bytes());

    if (0 == id->size()) { return false; }

    return Add(id);
}

auto Wallet::Accounts::init(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const internal::Network& network,
    const internal::WalletDatabase& db,
    const zmq::socket::Push& socket,
    const Type chain,
    JobCounter& jobs,
    const SimpleCallback& taskFinished) noexcept -> AccountMap
{
    auto output = AccountMap{};

    for (const auto& nym : api.Wallet().LocalNyms()) {
        LogNormal("Initializing ")(DisplayString(chain))(" wallet for ")(nym)
            .Flush();
        output.try_emplace(
            nym,
            api,
            blockchain,
            blockchain.BalanceTree(nym, chain),
            network,
            db,
            socket,
            jobs.Allocate(),
            taskFinished);
    }

    return output;
}

auto Wallet::Accounts::Reorg(const block::Position& parent) noexcept -> bool
{
    auto output{false};

    for (auto& [nym, account] : map_) { output |= account.reorg(parent); }

    return output;
}

auto Wallet::Accounts::state_machine() noexcept -> bool
{
    auto output{false};

    for (auto& [nym, account] : map_) { output |= account.state_machine(); }

    return output;
}
}  // namespace opentxs::blockchain::client::implementation
