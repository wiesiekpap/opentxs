// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/client/Blockchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <set>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

// #define OT_METHOD "opentxs::api::client::implementation::AccountCache::"

namespace opentxs::api::client::implementation
{
Blockchain::AccountCache::AccountCache(const api::Core& api) noexcept
    : api_(api)
    , lock_()
    , account_map_()
    , account_index_()
    , account_type_()
{
}

auto Blockchain::AccountCache::build_account_map(
    const Lock&,
    const Chain chain,
    std::optional<NymAccountMap>& map) const noexcept -> void
{
    const auto nyms = api_.Wallet().LocalNyms();
    map = NymAccountMap{};

    OT_ASSERT(map.has_value());

    auto& output = map.value();
    std::for_each(std::begin(nyms), std::end(nyms), [&](const auto& nym) {
        const auto hd =
            api_.Storage().BlockchainAccountList(nym->str(), Translate(chain));
        std::for_each(std::begin(hd), std::end(hd), [&](const auto& account) {
            auto& set = output[nym];
            auto accountID = api_.Factory().Identifier(account);
            account_index_.emplace(accountID, nym);
            account_type_.emplace(accountID, AccountType::HD);
            set.emplace(std::move(accountID));
        });
        const auto pc =
            api_.Storage().Bip47ChannelsByChain(nym, Translate(chain));
        std::for_each(std::begin(pc), std::end(pc), [&](const auto& accountID) {
            auto& set = output[nym];
            account_index_.emplace(accountID, nym);
            account_type_.emplace(accountID, AccountType::PaymentCode);
            set.emplace(std::move(accountID));
        });
    });
}

auto Blockchain::AccountCache::get_account_map(
    const Lock& lock,
    const Chain chain) const noexcept -> NymAccountMap&
{
    auto& map = account_map_[chain];

    if (false == map.has_value()) { build_account_map(lock, chain, map); }

    OT_ASSERT(map.has_value());

    return map.value();
}

auto Blockchain::AccountCache::List(
    const identifier::Nym& nymID,
    const Chain chain) const noexcept -> std::set<OTIdentifier>
{
    Lock lock(lock_);
    const auto& map = get_account_map(lock, chain);
    auto it = map.find(nymID);

    if (map.end() == it) { return {}; }

    return it->second;
}

auto Blockchain::AccountCache::New(
    const AccountType type,
    const Chain chain,
    const Identifier& account,
    const identifier::Nym& owner) const noexcept -> void
{
    Lock lock(lock_);
    get_account_map(lock, chain)[owner].emplace(account);
    account_index_.emplace(account, owner);
    account_type_.emplace(account, type);
}

auto Blockchain::AccountCache::Owner(const Identifier& accountID) const noexcept
    -> const identifier::Nym&
{
    static const auto blank = api_.Factory().NymID();

    try {

        return account_index_.at(accountID);
    } catch (...) {

        return blank;
    }
}

auto Blockchain::AccountCache::Populate() noexcept -> void
{
    Lock lock(lock_);

    for (const auto& chain : opentxs::blockchain::SupportedChains()) {
        auto& map = account_map_[chain];
        build_account_map(lock, chain, map);
    }
}

auto Blockchain::AccountCache::Type(const Identifier& accountID) const noexcept
    -> AccountType
{
    static const auto blank = api_.Factory().NymID();

    try {

        return account_type_.at(accountID);
    } catch (...) {

        return AccountType::Error;
    }
}
}  // namespace opentxs::api::client::implementation
