// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "opentxs/protobuf/BlockchainTransaction.pb.h"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
}  // namespace client

class Core;
class Legacy;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin

class Header;
}  // namespace block

namespace node
{
struct GCS;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace blockchain
{
namespace sync
{
class Block;
class Data;
}  // namespace sync
}  // namespace blockchain
}  // namespace network

namespace proto
{
class BlockchainTransaction;
}  // namespace proto

class Contact;
class Data;
class Options;
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Database final
{
public:
    enum class Key : std::size_t {
        BlockStoragePolicy = 0,
        NextBlockAddress = 1,
        SiphashKey = 2,
        NextSyncAddress = 3,
        SyncServerEndpoint = 4,
    };

    using BlockHash = opentxs::blockchain::block::Hash;
    using PatternID = opentxs::blockchain::PatternID;
    using Txid = opentxs::blockchain::block::Txid;
    using pTxid = opentxs::blockchain::block::pTxid;
    using Chain = opentxs::blockchain::Type;
    using EnabledChain = std::pair<Chain, std::string>;
    using Height = opentxs::blockchain::block::Height;
    using SyncItems = std::vector<opentxs::network::blockchain::sync::Block>;
    using Endpoints = std::vector<std::string>;

    auto AddOrUpdate(Address_p address) const noexcept -> bool;
    auto AddSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto AllocateStorageFolder(const std::string& dir) const noexcept
        -> std::string;
    auto AssociateTransaction(
        const Txid& txid,
        const std::vector<PatternID>& patterns) const noexcept -> bool;
    auto BlockHeaderExists(const BlockHash& hash) const noexcept -> bool;
    auto BlockExists(const BlockHash& block) const noexcept -> bool;
    auto BlockLoad(const BlockHash& block) const noexcept -> BlockReader;
    auto BlockPolicy() const noexcept -> BlockStorage;
    auto BlockStore(const BlockHash& block, const std::size_t bytes)
        const noexcept -> BlockWriter;
    auto DeleteSyncServer(const std::string& endpoint) const noexcept -> bool;
    auto Disable(const Chain type) const noexcept -> bool;
    auto Enable(const Chain type, const std::string& seednode) const noexcept
        -> bool;
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address_p;
    auto GetSyncServers() const noexcept -> Endpoints;
    auto HashKey() const noexcept -> ReadView;
    auto HaveFilter(const FilterType type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveFilterHeader(const FilterType type, const ReadView blockHash)
        const noexcept -> bool;
    auto Import(std::vector<Address_p> peers) const noexcept -> bool;
    auto LoadBlockHeader(const BlockHash& hash) const noexcept(false)
        -> proto::BlockchainBlockHeader;
    auto LoadEnabledChains() const noexcept -> std::vector<EnabledChain>;
    auto LoadFilter(const FilterType type, const ReadView blockHash)
        const noexcept -> std::unique_ptr<const opentxs::blockchain::node::GCS>;
    auto LoadFilterHash(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput filterHash) const noexcept -> bool;
    auto LoadFilterHeader(
        const FilterType type,
        const ReadView blockHash,
        const AllocateOutput header) const noexcept -> bool;
    auto LoadSync(
        const Chain chain,
        const Height height,
        opentxs::network::blockchain::sync::Data& output) const noexcept
        -> bool;
    auto LoadTransaction(const ReadView txid) const noexcept
        -> std::optional<proto::BlockchainTransaction>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier>;
    auto LookupTransactions(const PatternID pattern) const noexcept
        -> std::vector<pTxid>;
    auto ReorgSync(const Chain chain, const Height height) const noexcept
        -> bool;
    auto StoreBlockHeader(const opentxs::blockchain::block::Header& header)
        const noexcept -> bool;
    auto StoreBlockHeaders(const UpdatedHeader& headers) const noexcept -> bool;
    auto StoreFilterHeaders(
        const FilterType type,
        const std::vector<FilterHeader>& headers) const noexcept -> bool;
    auto StoreFilters(const FilterType type, std::vector<FilterData>& filters)
        const noexcept -> bool;
    auto StoreFilters(
        const FilterType type,
        const std::vector<FilterHeader>& headers,
        const std::vector<FilterData>& filters) const noexcept -> bool;
    auto StoreSync(const Chain chain, const SyncItems& items) const noexcept
        -> bool;
    auto StoreTransaction(const proto::BlockchainTransaction& tx) const noexcept
        -> bool;
    auto SyncTip(const Chain chain) const noexcept -> Height;
    auto UpdateContact(const Contact& contact) const noexcept
        -> std::vector<pTxid>;
    auto UpdateMergedContact(const Contact& parent, const Contact& child)
        const noexcept -> std::vector<pTxid>;

    Database(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const Options& args) noexcept(false);

    ~Database();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_p_;
    Imp& imp_;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database&&) -> Database& = delete;
};
}  // namespace opentxs::blockchain::database::common
