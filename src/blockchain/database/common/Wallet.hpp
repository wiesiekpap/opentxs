// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage

class Contact;
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Wallet
{
public:
    using PatternID = opentxs::blockchain::PatternID;
    using Transaction = proto::BlockchainTransaction;
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;

    auto AssociateTransaction(
        const Txid& txid,
        const std::vector<PatternID>& patterns) const noexcept -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::optional<Transaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> std::vector<pTxid>;
    auto StoreTransaction(const Transaction& tx) const noexcept -> bool;
    auto UpdateContact(const Contact& contact) const noexcept
        -> std::vector<pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> std::vector<pTxid>;

    Wallet(
        const api::client::Blockchain& blockchain,
        storage::lmdb::LMDB& lmdb,
        Bulk& bulk) noexcept(false);

    ~Wallet();

private:
    using ContactToElement = std::map<OTIdentifier, std::set<OTData>>;
    using ElementToContact = std::map<OTData, std::set<OTIdentifier>>;
    using TransactionToPattern = std::map<pTxid, std::set<PatternID>>;
    using PatternToTransaction = std::map<PatternID, std::set<pTxid>>;

    const api::client::Blockchain& blockchain_;
    storage::lmdb::LMDB& lmdb_;
    Bulk& bulk_;
    const int transaction_table_;
    mutable std::mutex lock_;
    mutable ContactToElement contact_to_element_;
    mutable ElementToContact element_to_contact_;
    mutable TransactionToPattern transaction_to_patterns_;
    mutable PatternToTransaction pattern_to_transactions_;

    auto update_contact(
        const Lock& lock,
        const std::set<OTData>& existing,
        const std::set<OTData>& incoming,
        const Identifier& contactID) const noexcept -> std::vector<pTxid>;
};
}  // namespace opentxs::blockchain::database::common
