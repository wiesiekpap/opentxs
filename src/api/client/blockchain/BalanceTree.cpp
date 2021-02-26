// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "api/client/blockchain/BalanceTree.tpp"  // IWYU pragma: associated

#include <algorithm>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "api/client/blockchain/BalanceTree.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "internal/api/client/blockchain/Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/blockchain/AddressStyle.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::implementation::BalanceTree::"

namespace opentxs::factory
{
auto BlockchainBalanceTree(
    const api::internal::Core& api,
    const api::client::blockchain::internal::BalanceList& parent,
    const identifier::Nym& id,
    const std::set<OTIdentifier>& hd,
    const std::set<OTIdentifier>& imported,
    const std::set<OTIdentifier>& pc) noexcept
    -> std::unique_ptr<api::client::blockchain::internal::BalanceTree>
{
    using ReturnType = api::client::blockchain::implementation::BalanceTree;

    return std::make_unique<ReturnType>(api, parent, id, hd, imported, pc);
}
}  // namespace opentxs::factory

namespace opentxs::api::client::blockchain::implementation
{
BalanceTree::BalanceTree(
    const api::internal::Core& api,
    const internal::BalanceList& parent,
    const identifier::Nym& nym,
    const Accounts& hd,
    const Accounts& imported,
    const Accounts& paymentCode) noexcept
    : api_(api)
    , parent_(parent)
    , chain_(parent.Chain())
    , nym_id_(nym)
    , hd_(api_, *this)
    , imported_(api_, *this)
    , payment_code_(api_, *this)
    , node_index_()
    , lock_()
    , unspent_()
    , spent_()
{
    init_hd(hd);
    init_payment_code(paymentCode);
}

auto BalanceTree::NodeIndex::Add(
    const std::string& id,
    internal::BalanceNode* node) noexcept -> void
{
    OT_ASSERT(nullptr != node);

    Lock lock(lock_);
    index_[id] = node;
}

auto BalanceTree::NodeIndex::Find(const std::string& id) const noexcept
    -> internal::BalanceNode*
{
    Lock lock(lock_);

    try {

        return index_.at(id);
    } catch (...) {

        return nullptr;
    }
}

auto BalanceTree::AssociateTransaction(
    const std::vector<Activity>& unspent,
    const std::vector<Activity>& spent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    using ActivityVector = std::vector<Activity>;
    using ActivityPair = std::pair<ActivityVector, ActivityVector>;
    using ActivityMap = std::map<std::string, ActivityPair>;

    Lock lock(lock_);
    auto sorted = ActivityMap{};
    auto outputs = std::map<std::string, std::map<std::size_t, int>>{};

    for (const auto& [coin, key, amount] : unspent) {
        const auto& [transaction, output] = coin;
        const auto& [account, subchain, index] = key;

        if (1 < ++outputs[transaction][output]) { return false; }
        if (0 >= amount) { return false; }

        sorted[account].first.emplace_back(Activity{coin, key, amount});
    }

    for (const auto& [coin, key, amount] : spent) {
        const auto& [transaction, output] = coin;
        const auto& [account, subchain, index] = key;

        if (1 < ++outputs[transaction][output]) { return false; }
        if (0 >= amount) { return false; }

        sorted[account].second.emplace_back(Activity{coin, key, amount});
    }

    for (const auto& [accountID, value] : sorted) {
        auto* pNode = node_index_.Find(accountID);

        if (nullptr == pNode) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Account ")(accountID)(
                " not found")
                .Flush();

            continue;
        }

        const auto& node = *pNode;
        const auto accepted = node.AssociateTransaction(
            value.first, value.second, contacts, reason);

        if (accepted) {
            for (const auto& [coin, key, amount] : value.first) {
                unspent_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(coin),
                    std::forward_as_tuple(key, amount));
            }

            for (const auto& [coin, key, amount] : value.second) {
                spent_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(coin),
                    std::forward_as_tuple(key, amount));
            }
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed processing transaction")
                .Flush();

            return false;
        }
    }

    return true;
}

auto BalanceTree::ClaimAccountID(
    const std::string& id,
    internal::BalanceNode* node) const noexcept -> void
{
    node_index_.Add(id, node);
}

auto BalanceTree::find_best_deposit_address() const noexcept -> const Element&
{
    try {
        const auto reason = api_.Factory().PasswordPrompt(__FUNCTION__);

        return find_next_element(Subchain::External, reason);
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": ")(e.what()).Flush();

        OT_FAIL;
    }
}

auto BalanceTree::find_next_element(
    Subchain subchain,
    const PasswordPrompt& reason) const noexcept(false) -> const Element&
{
    // TODO Add a mechanism for setting a default subaccount in case more than
    // one is present. Also handle cases where only an imported subaccount
    // exists

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunreachable-code-loop-increment"
    for (const auto& account : hd_) {
        const auto last = account.LastUsed(subchain);

        if (last.has_value()) {
#if OT_CRYPTO_WITH_BIP32
            const auto next = account.GenerateNext(subchain, reason);

            if (next.has_value()) {

                return account.BalanceElement(subchain, next.value());
            }
#else
            return account.BalanceElement(subchain, 0);
#endif
        } else {

            return account.BalanceElement(subchain, 0);
        }
    }
#pragma GCC diagnostic pop

    throw std::runtime_error("No available element for selected subchain");
}

auto BalanceTree::GetNextChangeKey(const PasswordPrompt& reason) const
    noexcept(false) -> const Element&
{
    return find_next_element(Subchain::Internal, reason);
}

auto BalanceTree::GetNextDepositKey(const PasswordPrompt& reason) const
    noexcept(false) -> const Element&
{
    return find_next_element(Subchain::External, reason);
}

auto BalanceTree::GetDepositAddress(const std::string& memo) const noexcept
    -> std::string
{
    const auto& element = find_best_deposit_address();

    if (false == memo.empty()) {
        parent_.Parent().AssignLabel(
            nym_id_,
            element.Parent().ID(),
            element.Subchain(),
            element.Index(),
            memo);
    }

    return element.Address(AddressStyle::P2PKH);  // TODO
}

auto BalanceTree::GetDepositAddress(
    const Identifier& contact,
    const std::string& memo) const noexcept -> std::string
{
    const auto& element = find_best_deposit_address();

    if (false == contact.empty()) {
        parent_.Parent().AssignContact(
            nym_id_,
            element.Parent().ID(),
            element.Subchain(),
            element.Index(),
            contact);
    }

    if (false == memo.empty()) {
        parent_.Parent().AssignLabel(
            nym_id_,
            element.Parent().ID(),
            element.Subchain(),
            element.Index(),
            memo);
    }

    return element.Address(AddressStyle::P2PKH);  // TODO
}

auto BalanceTree::init_hd(const Accounts& accounts) noexcept -> void
{
    for (const auto& accountID : accounts) {
        auto account = std::shared_ptr<proto::HDAccount>{};
        const auto loaded =
            api_.Storage().Load(nym_id_->str(), accountID->str(), account);

        if (false == loaded) { continue; }

        OT_ASSERT(account);

        auto notUsed = Identifier::Factory();
        hd_.Construct(notUsed, *account);
    }
}

auto BalanceTree::init_payment_code(const Accounts& accounts) noexcept -> void
{
    for (const auto& id : accounts) {
        auto account = std::shared_ptr<proto::Bip47Channel>{};
        const auto loaded = api_.Storage().Load(nym_id_, id, account);

        if (false == loaded) { continue; }

        OT_ASSERT(account);

        auto notUsed = Identifier::Factory();
        payment_code_.Construct(notUsed, *account);
    }
}

auto BalanceTree::LookupUTXO(const Coin& coin) const noexcept
    -> std::optional<std::pair<Key, Amount>>
{
    Lock lock(lock_);

    try {

        return unspent_.at(coin);
    } catch (...) {

        return {};
    }
}

auto BalanceTree::Node(const Identifier& id) const noexcept(false)
    -> internal::BalanceNode&
{
    auto* output = node_index_.Find(id.str());

    if (nullptr == output) { throw std::out_of_range("Account not found"); }

    return *output;
}
}  // namespace opentxs::api::client::blockchain::implementation
