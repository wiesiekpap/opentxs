// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/client/blockchain/database/Database.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <algorithm>
#include <cstring>
#include <map>
#include <stdexcept>
#include <utility>

#include "api/client/blockchain/database/BlockFilter.hpp"
#include "api/client/blockchain/database/BlockHeaders.hpp"
#if OPENTXS_BLOCK_STORAGE_ENABLED
#include "api/client/blockchain/database/Blocks.hpp"
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
#include "api/client/blockchain/database/Peers.hpp"
#if OPENTXS_BLOCK_STORAGE_ENABLED
#include "api/client/blockchain/database/Sync.hpp"
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
#include "api/client/blockchain/database/Wallet.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Legacy.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "util/LMDB.hpp"

// #define OT_METHOD
// "opentxs::api::client::blockchain::database::implementation::Database::"

namespace opentxs::api::client::blockchain::database::implementation
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

struct Database::Imp {
    using SiphashKey = Space;

    static const opentxs::storage::lmdb::TableNames table_names_;

    const api::Core& api_;
    const api::Legacy& legacy_;
    const OTString blockchain_path_;
    const OTString common_path_;
#if OPENTXS_BLOCK_STORAGE_ENABLED
    const OTString blocks_path_;
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    opentxs::storage::lmdb::LMDB lmdb_;
    const BlockStorage block_policy_;
    const SiphashKey siphash_key_;
    BlockHeader headers_;
    Peers peers_;
    BlockFilter filters_;
#if OPENTXS_BLOCK_STORAGE_ENABLED
    Blocks blocks_;
    Sync sync_;
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
    Wallet wallet_;

    static auto block_storage_enabled() noexcept -> bool
    {
        return 1 == OPENTXS_BLOCK_STORAGE_ENABLED;
    }
    static auto block_storage_level(
        const ArgList& args,
        opentxs::storage::lmdb::LMDB& lmdb) noexcept -> BlockStorage
    {
        if (false == block_storage_enabled()) { return BlockStorage::None; }

        auto output = block_storage_level_default();
        const auto arg = block_storage_level_arg(args);

        if (arg.has_value()) { output = arg.value(); }

        const auto db = block_storage_level_configured(lmdb);

        if (db.has_value()) { output = std::max(output, db.value()); }

        if ((false == db.has_value()) || (output != db.value())) {
            lmdb.Store(Config, tsv(Key::BlockStoragePolicy), tsv(output));
        }

        return output;
    }
    static auto block_storage_level_arg(const ArgList& args) noexcept
        -> std::optional<BlockStorage>
    {
        try {
            const auto& arg = args.at(OPENTXS_ARG_BLOCK_STORAGE_LEVEL);

            if (0 == arg.size()) { return BlockStorage::None; }

            switch (std::stoi(*arg.cbegin())) {
                case 2: {
                    return BlockStorage::All;
                }
                case 1: {
                    return BlockStorage::Cache;
                }
                default: {
                    return BlockStorage::None;
                }
            }
        } catch (...) {
            return {};
        }
    }
    static auto block_storage_level_configured(
        opentxs::storage::lmdb::LMDB& db) noexcept
        -> std::optional<BlockStorage>
    {
        if (false == db.Exists(Config, tsv(Key::BlockStoragePolicy))) {
            return std::nullopt;
        }

        auto output{BlockStorage::None};
        auto cb = [&output](const auto in) {
            if (sizeof(output) != in.size()) { return; }

            std::memcpy(&output, in.data(), in.size());
        };

        if (false == db.Load(Config, tsv(Key::BlockStoragePolicy), cb)) {
            return std::nullopt;
        }

        return output;
    }
    static auto block_storage_level_default() noexcept -> BlockStorage
    {
        if (2 == OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY) {

            return BlockStorage::All;
        } else if (1 == OPENTXS_DEFAULT_BLOCK_STORAGE_POLICY) {

            return BlockStorage::Cache;
        } else {

            return BlockStorage::None;
        }
    }
    static auto init_folder(
        const api::Legacy& legacy,
        const String& parent,
        const String& child) noexcept(false) -> OTString
    {
        auto output = String::Factory();

        if (false == legacy.AppendFolder(output, parent, child)) {
            throw std::runtime_error("Failed to calculate path");
        }

        if (false == legacy.BuildFolderPath(output)) {
            throw std::runtime_error("Failed to construct path");
        }

        return output;
    }
    static auto init_storage_path(
        const api::Legacy& legacy,
        const std::string& dataFolder) noexcept(false) -> OTString
    {
        return init_folder(
            legacy, String::Factory(dataFolder), String::Factory("blockchain"));
    }
    static auto siphash_key(opentxs::storage::lmdb::LMDB& db) noexcept
        -> SiphashKey
    {
        auto configured = siphash_key_configured(db);

        if (configured.has_value()) { return configured.value(); }

        auto output = space(crypto_shorthash_KEYBYTES);
        ::crypto_shorthash_keygen(
            reinterpret_cast<unsigned char*>(output.data()));
        const auto saved =
            db.Store(Config, tsv(Key::SiphashKey), reader(output));

        OT_ASSERT(saved.first);

        return output;
    }
    static auto siphash_key_configured(
        opentxs::storage::lmdb::LMDB& db) noexcept -> std::optional<SiphashKey>
    {
        if (false == db.Exists(Config, tsv(Key::SiphashKey))) {
            return std::nullopt;
        }

        auto output = space(crypto_shorthash_KEYBYTES);
        auto cb = [&output](const auto in) {
            if (output.size() != in.size()) { return; }

            std::memcpy(output.data(), in.data(), in.size());
        };

        if (false == db.Load(Config, tsv(Key::SiphashKey), cb)) {
            return std::nullopt;
        }

        return std::move(output);
    }

    auto AllocateStorageFolder(const std::string& dir) const noexcept
        -> std::string
    {
        return init_folder(legacy_, blockchain_path_, String::Factory(dir))
            ->Get();
    }

    Imp(const api::Core& api,
        const api::client::Blockchain& blockchain,
        const api::Legacy& legacy,
        const std::string& dataFolder,
        const ArgList& args) noexcept(false)
        : api_(api)
        , legacy_(legacy)
        , blockchain_path_(init_storage_path(legacy, dataFolder))
        , common_path_(
              init_folder(legacy, blockchain_path_, String::Factory("common")))
#if OPENTXS_BLOCK_STORAGE_ENABLED
        , blocks_path_(
              init_folder(legacy, common_path_, String::Factory("blocks")))
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
        , lmdb_(
              table_names_,
              common_path_->Get(),
              [] {
                  auto output = opentxs::storage::lmdb::TablesToInit{
                      {BlockHeaders, 0},
                      {PeerDetails, 0},
                      {PeerChainIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {PeerProtocolIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {PeerServiceIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {PeerNetworkIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {PeerConnectedIndex, MDB_DUPSORT | MDB_INTEGERKEY},
                      {FiltersBasic, 0},
                      {FiltersBCH, 0},
                      {FiltersOpentxs, 0},
                      {FilterHeadersBasic, 0},
                      {FilterHeadersBCH, 0},
                      {FilterHeadersOpentxs, 0},
                      {Config, MDB_INTEGERKEY},
                      {BlockIndex, 0},
                      {Enabled, MDB_INTEGERKEY},
                      {SyncTips, MDB_INTEGERKEY},
                  };

                  for (const auto& [table, name] : SyncTables()) {
                      output.emplace_back(table, MDB_INTEGERKEY);
                  }

                  return output;
              }())
        , block_policy_(block_storage_level(args, lmdb_))
        , siphash_key_(siphash_key(lmdb_))
        , headers_(api_, lmdb_)
        , peers_(api_, lmdb_)
        , filters_(api_, lmdb_)
#if OPENTXS_BLOCK_STORAGE_ENABLED
        , blocks_(lmdb_, blocks_path_->Get())
        , sync_(api_, lmdb_, blocks_path_->Get())
#endif  // OPENTXS_BLOCK_STORAGE_ENABLED
        , wallet_(blockchain, lmdb_)
    {
        OT_ASSERT(crypto_shorthash_KEYBYTES == siphash_key_.size());

        static_assert(
            sizeof(opentxs::blockchain::PatternID) == crypto_shorthash_BYTES);
    }
};

const opentxs::storage::lmdb::TableNames Database::Imp::table_names_ = [] {
    auto output = opentxs::storage::lmdb::TableNames{
        {BlockHeaders, "block_headers"},
        {PeerDetails, "peers"},
        {PeerChainIndex, "peer_chain_index"},
        {PeerProtocolIndex, "peer_protocol_index"},
        {PeerServiceIndex, "peer_service_index"},
        {PeerNetworkIndex, "peer_network_index"},
        {PeerConnectedIndex, "peer_connected_index"},
        {FiltersBasic, "block_filters_basic"},
        {FiltersBCH, "block_filters_bch"},
        {FiltersOpentxs, "block_filters_opentxs"},
        {FilterHeadersBasic, "block_filter_headers_basic"},
        {FilterHeadersBCH, "block_filter_headers_bch"},
        {FilterHeadersOpentxs, "block_filter_headers_opentxs"},
        {Config, "config"},
        {BlockIndex, "blocks"},
        {Enabled, "enabled_chains"},
        {SyncTips, "sync_tips"},
    };

    for (const auto& [table, name] : SyncTables()) {
        output.emplace(table, name);
    }

    return output;
}();

Database::Database(
    const api::Core& api,
    const api::client::Blockchain& blockchain,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const ArgList& args) noexcept(false)
    : imp_p_(std::make_unique<Imp>(api, blockchain, legacy, dataFolder, args))
    , imp_(*imp_p_)
{
    OT_ASSERT(imp_p_)
}

auto Database::AddOrUpdate(Address_p address) const noexcept -> bool
{
    return imp_.peers_.Insert(std::move(address));
}

auto Database::AllocateStorageFolder(const std::string& dir) const noexcept
    -> std::string
{
    return imp_.AllocateStorageFolder(dir);
}

auto Database::AssociateTransaction(
    const Txid& txid,
    const std::vector<PatternID>& patterns) const noexcept -> bool
{
    return imp_.wallet_.AssociateTransaction(txid, patterns);
}

auto Database::BlockHeaderExists(const BlockHash& hash) const noexcept -> bool
{
    return imp_.headers_.BlockHeaderExists(hash);
}

auto Database::BlockExists(const BlockHash& block) const noexcept -> bool
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return imp_.blocks_.Exists(block);
#else
    return false;
#endif
}

auto Database::BlockLoad(const BlockHash& block) const noexcept -> BlockReader
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return imp_.blocks_.Load(block);
#else
    return BlockReader{};
#endif
}

auto Database::BlockPolicy() const noexcept -> BlockStorage
{
    return imp_.block_policy_;
}

auto Database::BlockStore(const BlockHash& block, const std::size_t bytes)
    const noexcept -> BlockWriter
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return imp_.blocks_.Store(block, bytes);
#else
    return {};
#endif
}

auto Database::Disable(const Chain type) const noexcept -> bool
{
    static const auto data{false};
    const auto key = std::size_t{static_cast<std::uint32_t>(type)};

    return imp_.lmdb_.Store(Enabled, key, tsv(data)).first;
}

auto Database::Enable(const Chain type) const noexcept -> bool
{
    static const auto data{true};
    const auto key = std::size_t{static_cast<std::uint32_t>(type)};

    return imp_.lmdb_.Store(Enabled, key, tsv(data)).first;
}

auto Database::Find(
    const Chain chain,
    const Protocol protocol,
    const std::set<Type> onNetworks,
    const std::set<Service> withServices) const noexcept -> Address_p
{
    return imp_.peers_.Find(chain, protocol, onNetworks, withServices);
}

auto Database::HashKey() const noexcept -> ReadView
{
    return reader(imp_.siphash_key_);
}

auto Database::HaveFilter(const FilterType type, const ReadView blockHash)
    const noexcept -> bool
{
    return imp_.filters_.HaveFilter(type, blockHash);
}

auto Database::HaveFilterHeader(const FilterType type, const ReadView blockHash)
    const noexcept -> bool
{
    return imp_.filters_.HaveFilterHeader(type, blockHash);
}

auto Database::Import(std::vector<Address_p> peers) const noexcept -> bool
{
    return imp_.peers_.Import(std::move(peers));
}

auto Database::LoadBlockHeader(const BlockHash& hash) const noexcept(false)
    -> proto::BlockchainBlockHeader
{
    return imp_.headers_.LoadBlockHeader(hash);
}

auto Database::LoadFilter(const FilterType type, const ReadView blockHash)
    const noexcept -> std::unique_ptr<const opentxs::blockchain::client::GCS>
{
    return imp_.filters_.LoadFilter(type, blockHash);
}

auto Database::LoadFilterHash(
    const FilterType type,
    const ReadView blockHash,
    const AllocateOutput filterHash) const noexcept -> bool
{
    return imp_.filters_.LoadFilterHash(type, blockHash, filterHash);
}

auto Database::LoadFilterHeader(
    const FilterType type,
    const ReadView blockHash,
    const AllocateOutput header) const noexcept -> bool
{
    return imp_.filters_.LoadFilterHeader(type, blockHash, header);
}

auto Database::LoadTransaction(const ReadView txid) const noexcept
    -> std::optional<proto::BlockchainTransaction>
{
    return imp_.wallet_.LoadTransaction(txid);
}

auto Database::LookupContact(const Data& pubkeyHash) const noexcept
    -> std::set<OTIdentifier>
{
    return imp_.wallet_.LookupContact(pubkeyHash);
}

auto Database::LoadSync(
    const Chain chain,
    const Height height,
    opentxs::network::zeromq::Message& output) const noexcept -> bool
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return imp_.sync_.Load(chain, height, output);
#else
    return false;
#endif
}

auto Database::LookupTransactions(const PatternID pattern) const noexcept
    -> std::vector<pTxid>
{
    return imp_.wallet_.LookupTransactions(pattern);
}

auto Database::LoadEnabledChains() const noexcept -> std::vector<Chain>
{
    auto output = std::vector<Chain>{};
    const auto cb = [&](const auto key, const auto value) -> bool {
        auto chain = Chain{};
        auto enabled{false};
        std::memcpy(
            static_cast<void*>(&chain),
            key.data(),
            std::min(key.size(), sizeof(chain)));
        std::memcpy(
            static_cast<void*>(&enabled),
            value.data(),
            std::min(value.size(), sizeof(enabled)));

        if (enabled) { output.emplace_back(chain); }

        return true;
    };
    imp_.lmdb_.Read(Enabled, cb, opentxs::storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Database::ReorgSync(const Chain chain, const Height height) const noexcept
    -> bool
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return imp_.sync_.Reorg(chain, height);
#else
    return false;
#endif
}

auto Database::StoreBlockHeader(
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    return imp_.headers_.StoreBlockHeader(header);
}

auto Database::StoreBlockHeaders(const UpdatedHeader& headers) const noexcept
    -> bool
{
    return imp_.headers_.StoreBlockHeaders(headers);
}

auto Database::StoreFilterHeaders(
    const FilterType type,
    const std::vector<FilterHeader>& headers) const noexcept -> bool
{
    return imp_.filters_.StoreFilterHeaders(type, headers);
}

auto Database::StoreFilters(
    const FilterType type,
    std::vector<FilterData>& filters) const noexcept -> bool
{
    return imp_.filters_.StoreFilters(type, filters);
}

auto Database::StoreFilters(
    const FilterType type,
    const std::vector<FilterHeader>& headers,
    const std::vector<FilterData>& filters) const noexcept -> bool
{
    return imp_.filters_.StoreFilters(type, headers, filters);
}

auto Database::StoreSync(const Chain chain, const SyncItems& items)
    const noexcept -> bool
{
#if OPENTXS_BLOCK_STORAGE_ENABLED
    return imp_.sync_.Store(chain, items);
#else
    return false;
#endif
}

auto Database::StoreTransaction(
    const proto::BlockchainTransaction& tx) const noexcept -> bool
{
    return imp_.wallet_.StoreTransaction(tx);
}

auto Database::UpdateContact(const Contact& contact) const noexcept
    -> std::vector<pTxid>
{
    return imp_.wallet_.UpdateContact(contact);
}

auto Database::UpdateMergedContact(const Contact& parent, const Contact& child)
    const noexcept -> std::vector<pTxid>
{
    return imp_.wallet_.UpdateMergedContact(parent, child);
}

Database::~Database() = default;
}  // namespace opentxs::api::client::blockchain::database::implementation
