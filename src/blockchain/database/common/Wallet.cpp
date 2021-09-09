// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/common/Wallet.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/Container.hpp"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

#define OT_METHOD "opentxs::blockchain::database::common::Wallet::"

template <typename Input>
auto tsv(const Input& in) noexcept -> opentxs::ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

namespace opentxs::blockchain::database::common
{
Wallet::Wallet(
    const api::client::Blockchain& blockchain,
    storage::lmdb::LMDB& lmdb,
    Bulk& bulk) noexcept(false)
    : blockchain_(blockchain)
    , lmdb_(lmdb)
    , bulk_(bulk)
    , transaction_table_(Table::TransactionIndex)
    , lock_()
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
    LogTrace(OT_METHOD)(__func__)(": Transaction ")(txid.asHex())(
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
        LogTrace(OT_METHOD)(__func__)(": New patterns:").Flush();
    }

    std::for_each(
        std::begin(newElements),
        std::end(newElements),
        [&](const auto& element) {
            pattern_to_transactions_[element].insert(txid);
            LogTrace("    * ")(element).Flush();
        });

    if (0 < removedElements.size()) {
        LogTrace(OT_METHOD)(__func__)(": Obsolete patterns:").Flush();
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
    try {
        const auto index = [&] {
            auto out = util::IndexData{};
            auto cb = [&out](const ReadView in) {
                if (sizeof(out) != in.size()) { return; }

                std::memcpy(static_cast<void*>(&out), in.data(), in.size());
            };
            lmdb_.Load(transaction_table_, txid, cb);

            if (0 == out.size_) {
                throw std::out_of_range("Transaction not found");
            }

            return out;
        }();

        return proto::Factory<proto::BlockchainTransaction>(
            bulk_.ReadView(index));
    } catch (const std::exception& e) {
        LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return std::nullopt;
    }
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
    const proto::BlockchainTransaction& proto) const noexcept -> bool
{
    const auto& hash = proto.txid();

    try {
        const auto bytes = proto.ByteSizeLong();
        auto index = [&] {
            auto output = util::IndexData{};
            auto cb = [&output](const ReadView in) {
                if (sizeof(output) != in.size()) { return; }

                std::memcpy(static_cast<void*>(&output), in.data(), in.size());
            };
            lmdb_.Load(transaction_table_, hash, cb);

            return output;
        }();
        auto cb = [&](auto& tx) -> bool {
            const auto result =
                lmdb_.Store(transaction_table_, hash, tsv(index), tx);

            if (false == result.first) {
                LogOutput(OT_METHOD)(__func__)(
                    ": Failed to update index for transaction ")(hash)
                    .Flush();

                return false;
            }

            return true;
        };
        auto dLock = lmdb_.TransactionRW();
        auto view = [&] {
            auto bLock = Lock{bulk_.Mutex()};

            return bulk_.WriteView(bLock, dLock, index, std::move(cb), bytes);
        }();

        if (false == view.valid(bytes)) {
            throw std::runtime_error{
                "Failed to get write position for transaction"};
        }

        if (false == proto::write(proto, preallocated(bytes, view.data()))) {
            throw std::runtime_error{"Failed to write transaction to storage"};
        }

        if (false == dLock.Finalize(true)) {
            throw std::runtime_error{"Database update error"};
        }

        return true;
    } catch (const std::exception& e) {
        LogOutput(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return false;
    }
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
