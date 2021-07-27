// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/crypto/Wallet.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <string>
#include <utility>

#include "blockchain/crypto/AccountIndex.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/crypto/Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs::factory
{
auto BlockchainWalletKeys(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& parent,
    const api::client::internal::BalanceTreeIndex& index,
    const blockchain::Type chain) noexcept
    -> std::unique_ptr<blockchain::crypto::internal::Wallet>
{
    using ReturnType = blockchain::crypto::implementation::Wallet;

    return std::make_unique<ReturnType>(api, parent, index, chain);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::crypto::implementation
{
Wallet::Wallet(
    const api::internal::Core& api,
    const api::client::internal::Blockchain& parent,
    const api::client::internal::BalanceTreeIndex& index,
    const opentxs::blockchain::Type chain) noexcept
    : parent_(parent)
    , account_index_(index)
    , api_(api)
    , chain_(chain)
    , lock_()
    , trees_()
    , index_()
{
    init();
}

auto Wallet::add(
    const Lock& lock,
    const identifier::Nym& id,
    std::unique_ptr<internal::Account> tree) noexcept -> bool
{
    if (false == bool(tree)) { return false; }

    if (0 < index_.count(id)) { return false; }

    trees_.emplace_back(std::move(tree));
    const std::size_t position = trees_.size() - 1;
    index_.emplace(id, position);

    return true;
}

auto Wallet::AddHDNode(
    const identifier::Nym& nym,
    const proto::HDPath& path,
    const PasswordPrompt& reason,
    Identifier& id) noexcept -> bool
{
    Lock lock(lock_);

    return get_or_create(lock, nym).AddHDNode(path, reason, id);
}

auto Wallet::at(const std::size_t position) const noexcept(false)
    -> Wallet::const_iterator::value_type&
{
    Lock lock(lock_);

    return at(lock, position);
}

auto Wallet::at(const Lock& lock, const std::size_t index) const noexcept(false)
    -> const internal::Account&
{
    return *trees_.at(index);
}

auto Wallet::at(const Lock& lock, const std::size_t index) noexcept(false)
    -> internal::Account&
{
    return *trees_.at(index);
}

auto Wallet::factory(
    const identifier::Nym& nym,
    const Accounts& hd,
    const Accounts& paymentCode) const noexcept
    -> std::unique_ptr<internal::Account>
{
    return factory::BlockchainAccountKeys(
        api_, *this, account_index_, nym, hd, {}, paymentCode);
}

auto Wallet::get_or_create(const Lock& lock, const identifier::Nym& id) noexcept
    -> internal::Account&
{
    if (0 == index_.count(id)) {
        auto pTree = factory(id, {}, {});

        OT_ASSERT(pTree);

        const auto added = add(lock, id, std::move(pTree));

        OT_ASSERT(added);
    }

    return at(lock, index_.at(id));
}

void Wallet::init() noexcept
{
    Lock lock(lock_);
    const auto nyms = api_.Storage().LocalNyms();

    for (const auto& id : nyms) {
        const auto nymID = api_.Factory().NymID(id);
        const auto hdAccounts = [&] {
            auto out = Accounts{};
            const auto list =
                api_.Storage().BlockchainAccountList(id, Translate(chain_));
            std::transform(
                list.begin(),
                list.end(),
                std::inserter(out, out.end()),
                [&](const auto& in) { return api_.Factory().Identifier(in); });

            return out;
        }();
        const auto pcAccounts =
            api_.Storage().Bip47ChannelsByChain(nymID, Translate(chain_));

        add(lock, nymID, factory(nymID, hdAccounts, pcAccounts));
    }
}

auto Wallet::Nym(const identifier::Nym& id) noexcept -> internal::Account&
{
    Lock lock(lock_);

    return get_or_create(lock, id);
}
}  // namespace opentxs::blockchain::crypto::implementation
