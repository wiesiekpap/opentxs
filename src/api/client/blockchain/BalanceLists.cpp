// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "api/client/blockchain/BalanceLists.hpp"  // IWYU pragma: associated

#include <map>
#include <utility>

#include "internal/api/Api.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/api/client/blockchain/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"

// #define OT_METHOD
// "opentxs::api::client::implementation::BalanceLists::"

namespace opentxs::api::client::implementation
{
BalanceLists::BalanceLists(
    const api::internal::Core& api,
    api::client::internal::Blockchain& parent) noexcept
    : api_(api)
    , parent_(parent)
    , index_(api_)
    , lock_()
    , populated_(false)
    , lists_()
{
}

auto BalanceLists::AccountList(const identifier::Nym& nym) const noexcept
    -> std::set<OTIdentifier>
{
    populate();

    return index_.AccountList(nym);
}

auto BalanceLists::AccountList(const Chain chain) const noexcept
    -> std::set<OTIdentifier>
{
    populate();

    return index_.AccountList(chain);
}

auto BalanceLists::AccountList() const noexcept -> std::set<OTIdentifier>
{
    populate();

    return index_.AccountList();
}

auto BalanceLists::Get(const Chain chain) noexcept
    -> client::blockchain::internal::BalanceList&
{
    auto lock = Lock{lock_};

    return get(lock, chain);
}

auto BalanceLists::get(const Lock& lock, const Chain chain) const noexcept
    -> client::blockchain::internal::BalanceList&
{
    auto it = lists_.find(chain);

    if (lists_.end() != it) { return *it->second; }

    auto [it2, added] = lists_.emplace(
        chain, factory::BlockchainBalanceList(api_, parent_, index_, chain));

    OT_ASSERT(added);
    OT_ASSERT(it2->second);

    return *it2->second;
}

auto BalanceLists::LookupAccount(const Identifier& id) const noexcept
    -> AccountData
{
    populate();

    return index_.Query(id);
}

auto BalanceLists::populate() const noexcept -> void
{
    auto lock = Lock{lock_};

    if (populated_) { return; }

    for (const auto chain : opentxs::blockchain::SupportedChains()) {
        get(lock, chain);
    }

    populated_ = true;
}
}  // namespace opentxs::api::client::implementation
