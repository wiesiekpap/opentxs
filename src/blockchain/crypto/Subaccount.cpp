// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/crypto/Subaccount.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "internal/contact/Contact.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/key/HD.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/BlockchainAccountData.pb.h"
#include "opentxs/protobuf/BlockchainActivity.pb.h"

// #define OT_METHOD
// "opentxs::blockchain::crypto::implementation::Subaccount::"

namespace opentxs::blockchain::crypto::implementation
{
Subaccount::AddressData::AddressData(
    const api::Core& api,
    Subchain type,
    bool contact) noexcept
    : type_(type)
    , set_contact_(contact)
    , progress_(-1, block::BlankHash())
    , map_()
{
}

Subaccount::Subaccount(
    const api::Core& api,
    const Account& parent,
    const SubaccountType type,
    OTIdentifier&& id,
    const Revision revision,
    const std::vector<Activity>& unspent,
    const std::vector<Activity>& spent,
    Identifier& out) noexcept
    : api_(api)
    , parent_(parent)
    , chain_(parent_.Chain())
    , type_(type)
    , id_(std::move(id))
    , lock_()
    , revision_(revision)
    , unspent_(convert(unspent))
    , spent_(convert(spent))
{
    out.Assign(id_);
}

Subaccount::Subaccount(
    const api::Core& api,
    const Account& parent,
    const SubaccountType type,
    OTIdentifier&& id,
    Identifier& out) noexcept
    : Subaccount(api, parent, type, std::move(id), 0, {}, {}, out)
{
}

Subaccount::Subaccount(
    const api::Core& api,
    const Account& parent,
    const SubaccountType type,
    const SerializedType& serialized,
    Identifier& out) noexcept(false)
    : Subaccount(
          api,
          parent,
          type,
          api.Factory().Identifier(serialized.id()),
          serialized.revision(),
          convert(serialized.unspent()),
          convert(serialized.spent()),
          out)
{
    if (Translate(contact::internal::translate(serialized.chain())) != chain_) {
        throw std::runtime_error("Wrong account type");
    }
}

auto Subaccount::AssociateTransaction(
    const std::vector<Activity>& unspent,
    const std::vector<Activity>& spent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    auto lock = rLock{lock_};

    if (false == check_activity(lock, unspent, contacts, reason)) {

        return false;
    }

    for (const auto& [coin, key, value] : spent) {
        process_spent(lock, coin, key, value);
    }

    for (const auto& [coin, key, value] : unspent) {
        process_unspent(lock, coin, key, value);
    }

    return save(lock);
}

auto Subaccount::Confirm(
    const Subchain type,
    const Bip32Index index,
    const Txid& tx) noexcept -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);

        if (element.Internal().Confirm(tx)) {
            confirm(lock, type, index);

            return save(lock);
        } else {

            return false;
        }
    } catch (...) {
        return false;
    }
}

auto Subaccount::convert(Activity&& in) noexcept -> proto::BlockchainActivity
{
    const auto& [coin, key, value] = in;
    const auto& [txid, out] = coin;
    const auto& [account, chain, index] = key;
    proto::BlockchainActivity output{};
    output.set_version(ActivityVersion);
    output.set_txid(txid);
    output.set_output(out);
    output.set_amount(value);
    output.set_account(account);
    output.set_subchain(static_cast<std::uint32_t>(chain));
    output.set_index(index);

    return output;
}

auto Subaccount::convert(const proto::BlockchainActivity& in) noexcept
    -> Activity
{
    Activity output{};
    auto& [coin, key, value] = output;
    auto& [txid, out] = coin;
    auto& [account, chain, index] = key;
    txid = in.txid();
    out = in.output();
    value = in.amount();
    account = in.account();
    chain = static_cast<Subchain>(in.subchain());
    index = in.index();

    return output;
}

auto Subaccount::convert(const SerializedActivity& in) noexcept
    -> std::vector<Activity>
{
    auto output = std::vector<Activity>{};

    for (const auto& activity : in) { output.emplace_back(convert(activity)); }

    return output;
}

auto Subaccount::convert(const std::vector<Activity>& in) noexcept
    -> internal::ActivityMap
{
    auto output = internal::ActivityMap{};

    for (const auto& [coin, key, value] : in) {
        output.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(coin),
            std::forward_as_tuple(key, value));
    }

    return output;
}

auto Subaccount::IncomingTransactions(const Key& element) const noexcept
    -> std::set<std::string>
{
    auto lock = rLock{lock_};
    auto output = std::set<std::string>{};

    for (const auto& [coin, data] : unspent_) {
        const auto& [key, amount] = data;

        if (key == element) { output.emplace(coin.first); }
    }

    for (const auto& [coin, data] : spent_) {
        const auto& [key, amount] = data;

        if (key == element) { output.emplace(coin.first); }
    }

    return output;
}

void Subaccount::init() noexcept
{
    parent_.Internal().ClaimAccountID(id_->str(), this);
}

// Due to asynchronous blockchain scanning, spends may be discovered out of
// order compared to receipts.
void Subaccount::process_spent(
    const rLock& lock,
    const Coin& coin,
    const Key key,
    const Amount value) const noexcept
{
    auto targetValue{value};

    if (0 < unspent_.count(coin)) {
        // Normal case
        targetValue = std::max(targetValue, unspent_.at(coin).second);
        unspent_.erase(coin);
    }

    // If the spend was found before the receipt, the value is not known
    spent_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(coin),
        std::forward_as_tuple(key, targetValue));
}

void Subaccount::process_unspent(
    const rLock& lock,
    const Coin& coin,
    const Key key,
    const Amount value) const noexcept
{
    if (0 == spent_.count(coin)) {
        // Normal case
        unspent_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(coin),
            std::forward_as_tuple(key, value));
    } else {
        // Spend was discovered out of order, so correct the value now
        auto& storedValue = spent_.at(coin).second;
        storedValue = std::max(storedValue, value);
    }
}

auto Subaccount::ScanProgress(Subchain type) const noexcept -> block::Position
{
    static const auto blank = block::Position{-1, api_.Factory().Data()};

    return blank;
}

auto Subaccount::serialize_common(
    const rLock&,
    proto::BlockchainAccountData& out) const noexcept -> void
{
    out.set_version(BlockchainAccountDataVersion);
    out.set_id(id_->str());
    out.set_revision(revision_.load());
    out.set_chain(contact::internal::translate(Translate(chain_)));

    for (const auto& [coin, data] : unspent_) {
        auto converted = Activity{coin, data.first, data.second};
        *out.add_unspent() = convert(std::move(converted));
    }

    for (const auto& [coin, data] : spent_) {
        auto converted = Activity{coin, data.first, data.second};
        *out.add_spent() = convert(std::move(converted));
    }
}

auto Subaccount::SetContact(
    const Subchain type,
    const Bip32Index index,
    const Identifier& id) noexcept(false) -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);
        element.Internal().SetContact(id);

        return save(lock);
    } catch (...) {
        return false;
    }
}

auto Subaccount::SetLabel(
    const Subchain type,
    const Bip32Index index,
    const std::string& label) noexcept(false) -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);
        element.Internal().SetLabel(label);

        return save(lock);
    } catch (...) {
        return false;
    }
}

auto Subaccount::Unconfirm(
    const Subchain type,
    const Bip32Index index,
    const Txid& tx,
    const Time time) noexcept -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);

        if (element.Internal().Unconfirm(tx, time)) {
            unconfirm(lock, type, index);

            return save(lock);
        } else {

            return false;
        }
    } catch (...) {
        return false;
    }
}

auto Subaccount::Unreserve(const Subchain type, const Bip32Index index) noexcept
    -> bool
{
    auto lock = rLock{lock_};

    try {
        auto& element = mutable_element(lock, type, index);

        if (element.Internal().Unreserve()) {

            return save(lock);
        } else {

            return false;
        }
    } catch (...) {
        return false;
    }
}

auto Subaccount::UpdateElement(
    std::vector<ReadView>& pubkeyHashes) const noexcept -> void
{
    parent_.Parent().Parent().Internal().UpdateElement(pubkeyHashes);
}
}  // namespace opentxs::blockchain::crypto::implementation
