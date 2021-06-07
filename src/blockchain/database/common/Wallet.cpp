// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/common/Wallet.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::blockchain::database::common::Wallet::"

namespace opentxs::blockchain::database::common
{
Wallet::Wallet(
    const api::client::Blockchain& blockchain,
    [[maybe_unused]] opentxs::storage::lmdb::LMDB& lmdb) noexcept(false)
    : blockchain_(blockchain)
    // TODO , lmdb_(lmdb)
    , lock_()
    , transaction_map_()
    , contact_to_element_()
    , element_to_contact_()
    , transaction_to_patterns_()
    , pattern_to_transactions_()
{
}

auto Wallet::AssociateTransaction(
    const Txid& txid,
    const std::vector<PatternID>& in) const noexcept -> bool
{
    LogTrace(OT_METHOD)(__FUNCTION__)(": Transaction ")(txid.asHex())(
        " is associated with patterns:")
        .Flush();
    // TODO transaction data never changes so indexing should only happen
    // once.
    auto incoming = std::set<PatternID>{};
    std::for_each(std::begin(in), std::end(in), [&](auto& pattern) {
        incoming.emplace(pattern);
        LogTrace("    * ")(pattern).Flush();
    });
    Lock lock(lock_);
    auto& existing = transaction_to_patterns_[txid];
    auto newElements = std::vector<PatternID>{};
    auto removedElements = std::vector<PatternID>{};
    std::set_difference(
        std::begin(incoming),
        std::end(incoming),
        std::begin(existing),
        std::end(existing),
        std::back_inserter(newElements));
    std::set_difference(
        std::begin(existing),
        std::end(existing),
        std::begin(incoming),
        std::end(incoming),
        std::back_inserter(removedElements));

    if (0 < newElements.size()) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": New patterns:").Flush();
    }

    std::for_each(
        std::begin(newElements),
        std::end(newElements),
        [&](const auto& element) {
            pattern_to_transactions_[element].insert(txid);
            LogTrace("    * ")(element).Flush();
        });

    if (0 < removedElements.size()) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Obsolete patterns:").Flush();
    }

    std::for_each(
        std::begin(removedElements),
        std::end(removedElements),
        [&](const auto& element) {
            pattern_to_transactions_[element].erase(txid);
            LogTrace("    * ")(element).Flush();
        });
    existing.swap(incoming);

    return true;
}

auto Wallet::LoadTransaction(const ReadView txid) const noexcept
    -> std::optional<proto::BlockchainTransaction>
{
    Lock lock(lock_);
    const auto it = transaction_map_.find(std::string{txid});

    if (transaction_map_.end() == it) { return std::nullopt; }

    return it->second;
}

auto Wallet::LookupContact(const Data& pubkeyHash) const noexcept
    -> std::set<OTIdentifier>
{
    Lock lock(lock_);

    return element_to_contact_[pubkeyHash];
}

auto Wallet::LookupTransactions(const PatternID pattern) const noexcept
    -> std::vector<pTxid>
{
    auto output = std::vector<pTxid>{};

    try {
        const auto& data = pattern_to_transactions_.at(pattern);
        std::transform(
            std::begin(data), std::end(data), std::back_inserter(output), [
            ](const auto& txid) -> auto { return txid; });

    } catch (...) {
    }

    return output;
}

auto Wallet::StoreTransaction(
    const proto::BlockchainTransaction& tx) const noexcept -> bool
{
    Lock lock(lock_);
    transaction_map_[tx.txid()] = tx;

    return true;
}

auto Wallet::update_contact(
    const Lock& lock,
    const std::set<OTData>& existing,
    const std::set<OTData>& incoming,
    const Identifier& contactID) const noexcept -> std::vector<pTxid>
{
    auto newAddresses = std::vector<OTData>{};
    auto removedAddresses = std::vector<OTData>{};
    auto output = std::vector<pTxid>{};
    std::set_difference(
        std::begin(incoming),
        std::end(incoming),
        std::begin(existing),
        std::end(existing),
        std::back_inserter(newAddresses));
    std::set_difference(
        std::begin(existing),
        std::end(existing),
        std::begin(incoming),
        std::end(incoming),
        std::back_inserter(removedAddresses));
    std::for_each(
        std::begin(removedAddresses),
        std::end(removedAddresses),
        [&](const auto& element) {
            element_to_contact_[element].erase(contactID);
            const auto pattern = blockchain_.IndexItem(element->Bytes());

            try {
                const auto& transactions = pattern_to_transactions_.at(pattern);
                std::copy(
                    std::begin(transactions),
                    std::end(transactions),
                    std::back_inserter(output));
            } catch (...) {
            }
        });
    std::for_each(
        std::begin(newAddresses),
        std::end(newAddresses),
        [&](const auto& element) {
            element_to_contact_[element].insert(contactID);
            const auto pattern = blockchain_.IndexItem(element->Bytes());

            try {
                const auto& transactions = pattern_to_transactions_.at(pattern);
                std::copy(
                    std::begin(transactions),
                    std::end(transactions),
                    std::back_inserter(output));
            } catch (...) {
            }
        });
    dedup(output);

    return output;
}

auto Wallet::UpdateContact(const Contact& contact) const noexcept
    -> std::vector<pTxid>
{
    auto incoming = std::set<OTData>{};

    {
        auto data = contact.BlockchainAddresses();
        std::for_each(std::begin(data), std::end(data), [&](auto& in) {
            auto& [bytes, style, type] = in;
            incoming.emplace(std::move(bytes));
        });
    }

    Lock lock(lock_);
    const auto& contactID = contact.ID();
    auto& existing = contact_to_element_[contactID];
    auto output = update_contact(lock, existing, incoming, contactID);
    existing.swap(incoming);

    return output;
}

auto Wallet::UpdateMergedContact(const Contact& parent, const Contact& child)
    const noexcept -> std::vector<pTxid>
{
    auto deleted = std::set<OTData>{};
    auto incoming = std::set<OTData>{};

    {
        auto data = child.BlockchainAddresses();
        std::for_each(std::begin(data), std::end(data), [&](auto& in) {
            auto& [bytes, style, type] = in;
            deleted.emplace(std::move(bytes));
        });
    }

    {
        auto data = parent.BlockchainAddresses();
        std::for_each(std::begin(data), std::end(data), [&](auto& in) {
            auto& [bytes, style, type] = in;
            incoming.emplace(std::move(bytes));
        });
    }

    Lock lock(lock_);
    const auto& contactID = parent.ID();
    const auto& deletedID = child.ID();
    auto& existing = contact_to_element_[contactID];
    contact_to_element_.erase(deletedID);
    auto output = update_contact(lock, existing, incoming, contactID);
    std::for_each(
        std::begin(deleted), std::end(deleted), [&](const auto& element) {
            element_to_contact_[element].erase(deletedID);
            const auto pattern = blockchain_.IndexItem(element->Bytes());

            try {
                const auto& transactions = pattern_to_transactions_.at(pattern);
                std::copy(
                    std::begin(transactions),
                    std::end(transactions),
                    std::back_inserter(output));
            } catch (...) {
            }
        });
    dedup(output);
    existing.swap(incoming);

    return output;
}

Wallet::~Wallet() = default;
}  // namespace opentxs::blockchain::database::common
