// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "blockchain/database/Headers.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

#include "Proto.tpp"
#include "blockchain/database/common/Database.hpp"
#include "blockchain/node/UpdateTransaction.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Header.hpp"  // IWYU pragma: keep
#include "internal/blockchain/block/Factory.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "serialization/protobuf/BlockchainBlockLocalData.pb.h"
#include "util/LMDB.hpp"
#include "util/Work.hpp"
#include "util/threadutil.hpp"

namespace opentxs::blockchain::database
{
Headers::Headers(
    const api::Session& api,
    const node::internal::Manager& network,
    const common::Database& common,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , network_(network)
    , common_(common)
    , lmdb_(lmdb)
    , lock_()
{
    import_genesis(type);

    {
        const auto best = this->best();

        OT_ASSERT(HeaderExists(best.second));
        OT_ASSERT(0 <= best.first);
    }

    {
        const auto header = CurrentBest();

        OT_ASSERT(header);
        OT_ASSERT(0 <= header->Position().first);
    }
}

auto Headers::ApplyUpdate(const node::UpdateTransaction& update) noexcept
    -> bool
{
    if (!common_.StoreBlockHeaders(update.UpdatedHeaders())) {
        LogError()(OT_PRETTY_CLASS())("Failed to save block headers").Flush();

        std::cerr << ThreadMonitor::get_name()
                  << " Failed to save block headers\n";
        return false;
    }

    Lock lock(lock_);
    const auto initialHeight = best(lock).first;
    auto parentTxn = lmdb_.TransactionRW();

    if (update.HaveCheckpoint()) {
        if (!lmdb_
                 .Store(
                     ChainData,
                     tsv(static_cast<std::size_t>(Key::CheckpointHeight)),
                     tsv(static_cast<std::size_t>(update.Checkpoint().first)),
                     parentTxn)
                 .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save checkpoint height")
                .Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to save checkpoint height\n";
            return false;
        }

        if (!lmdb_
                 .Store(
                     ChainData,
                     tsv(static_cast<std::size_t>(Key::CheckpointHash)),
                     update.Checkpoint().second.Bytes(),
                     parentTxn)
                 .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save checkpoint hash")
                .Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to save checkpoint hash\n";
            return false;
        }
    }

    for (const auto& [parent, child] : update.Disconnected()) {
        if (!lmdb_
                 .Store(
                     BlockHeaderDisconnected,
                     parent.Bytes(),
                     child.Bytes(),
                     parentTxn)
                 .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save disconnected hash")
                .Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to save disconnected hash\n";
            return false;
        }
    }

    for (const auto& [parent, child] : update.Connected()) {
        if (!lmdb_.Delete(
                BlockHeaderDisconnected,
                parent.Bytes(),
                child.Bytes(),
                parentTxn)) {
            LogError()(OT_PRETTY_CLASS())("Failed to delete disconnected hash")
                .Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to delete disconnected hash\n";
            return false;
        }
    }

    for (const auto& hash : update.SiblingsToAdd()) {
        if (!lmdb_
                 .Store(
                     BlockHeaderSiblings, hash.Bytes(), hash.Bytes(), parentTxn)
                 .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save sibling hash")
                .Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to save sibling hash\n";
            return false;
        }
    }

    for (const auto& hash : update.SiblingsToDelete()) {
        lmdb_.Delete(BlockHeaderSiblings, hash.Bytes(), parentTxn);
    }

    for (const auto& data : update.UpdatedHeaders()) {
        const auto& [hash, pair] = data;
        block::internal::Header::SerializedType serializedType{};
        data.second.first->Internal().Serialize(serializedType);

        const auto result = lmdb_.Store(
            BlockHeaderMetadata,
            hash.Bytes(),
            proto::ToString(serializedType.local()),
            parentTxn);

        if (!result.first) {
            LogError()(OT_PRETTY_CLASS())("Failed to save block metadata")
                .Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to save block metadata\n";
            return false;
        }
    }

    if (update.HaveReorg()) {
        for (auto i = initialHeight; i > update.ReorgParent().first; --i) {
            if (!pop_best(i, parentTxn)) {
                LogError()(OT_PRETTY_CLASS())("Failed to delete best hash")
                    .Flush();

                std::cerr << ThreadMonitor::get_name()
                          << " Failed to delete best hash\n";
                return false;
            }
        }
    }

    for (const auto& position : update.BestChain()) {
        push_best(position, false, parentTxn);
    }

    if (!update.BestChain().empty()) {
        const auto& tip = *update.BestChain().crbegin();

        if (!lmdb_
                 .Store(
                     ChainData,
                     tsv(static_cast<std::size_t>(Key::TipHeight)),
                     tsv(static_cast<std::size_t>(tip.first)),
                     parentTxn)
                 .first) {
            LogError()(OT_PRETTY_CLASS())("Failed to store best hash").Flush();

            std::cerr << ThreadMonitor::get_name()
                      << " Failed to store best hash\n";
            return false;
        }
    }

    if (!parentTxn.Finalize(true)) {
        LogError()(OT_PRETTY_CLASS())("Database error").Flush();

        std::cerr << ThreadMonitor::get_name() << " Database error\n";
        return false;
    }

    const auto position = best(lock);
    const auto& [height, hash] = position;
    const auto bytes = hash.Bytes();

    if (update.HaveReorg()) {
        const auto [pHeight, pHash] = update.ReorgParent();
        const auto pBytes = pHash.Bytes();
        LogConsole()(print(network_.Chain()))(
            " reorg detected. Last common ancestor is ")(pHash.asHex())(
            " at height ")(pHeight)
            .Flush();
        auto work = MakeWork(WorkType::BlockchainReorg);
        work.AddFrame(network_.Chain());
        work.AddFrame(pBytes.data(), pBytes.size());
        work.AddFrame(pHeight);
        work.AddFrame(bytes.data(), bytes.size());
        work.AddFrame(height);

        std::cerr << ThreadMonitor::get_name()
                  << " Headers::ApplyUpdate sending BlockchainReorg\n";
        MessageMarker().mark(work);
        network_.Reorg().Send(std::move(work));
    } else {
        auto work = MakeWork(WorkType::BlockchainNewHeader);
        work.AddFrame(network_.Chain());
        work.AddFrame(bytes.data(), bytes.size());
        work.AddFrame(height);
        std::cerr << ThreadMonitor::get_name()
                  << " Headers::ApplyUpdate sending BlockchainNewHeader\n";

        // QQQ
        MessageMarker().mark(work);
        network_.Reorg().Send(std::move(work));
    }

    network_.UpdateLocalHeight(position);

    return true;
}

auto Headers::BestBlock(const block::Height position) const noexcept(false)
    -> block::Hash
{
    auto output = block::Hash{};

    if (0 > position) { return output; }

    lmdb_.Load(
        BlockHeaderBest,
        tsv(static_cast<std::size_t>(position)),
        [&](const auto in) -> void {
            const auto rc = output.Assign(in.data(), in.size());

            if (!rc) {
                throw std::runtime_error("Database contains invalid hash");
            }
        });

    if (output.IsNull()) {
        // TODO some callers which should be catching this exception aren't.
        // Clean up those call sites then start throwing this exception.
        // throw std::out_of_range("No best hash at specified height");
    }

    return output;
}

auto Headers::best() const noexcept -> block::Position
{
    Lock lock(lock_);

    return best(lock);
}

auto Headers::best(const Lock& lock) const noexcept -> block::Position
{
    auto output = make_blank<block::Position>::value(api_);
    auto height = std::size_t{0};

    if (!lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::TipHeight)),
            [&](const auto in) -> void {
                std::memcpy(
                    &height, in.data(), std::min(in.size(), sizeof(height)));
            })) {

        return make_blank<block::Position>::value(api_);
    }

    if (!lmdb_.Load(BlockHeaderBest, tsv(height), [&](const auto in) -> void {
            const auto rc = output.second.Assign(in.data(), in.size());

            OT_ASSERT(rc);  // TODO exception
        })) {

        return make_blank<block::Position>::value(api_);
    }

    output.first = height;

    return output;
}

auto Headers::checkpoint(const Lock& lock) const noexcept -> block::Position
{
    auto output = make_blank<block::Position>::value(api_);
    auto height = std::size_t{0};

    if (!lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::CheckpointHeight)),
            [&](const auto in) -> void {
                std::memcpy(
                    &height, in.data(), std::min(in.size(), sizeof(height)));
            })) {
        return make_blank<block::Position>::value(api_);
    }

    if (!lmdb_.Load(
            ChainData,
            tsv(static_cast<std::size_t>(Key::CheckpointHash)),
            [&](const auto in) -> void {
                const auto rc = output.second.Assign(in.data(), in.size());

                OT_ASSERT(rc);  // TODO exception
            })) {

        return make_blank<block::Position>::value(api_);
    }

    output.first = height;

    return output;
}

auto Headers::CurrentCheckpoint() const noexcept -> block::Position
{
    Lock lock(lock_);

    return checkpoint(lock);
}

auto Headers::DisconnectedHashes() const noexcept -> database::DisconnectedList
{
    Lock lock(lock_);
    auto output = database::DisconnectedList{};
    lmdb_.Read(
        BlockHeaderDisconnected,
        [&](const auto key, const auto value) -> bool {
            output.emplace(block::Hash{key}, block::Hash{value});

            return true;
        },
        storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Headers::HasDisconnectedChildren(const block::Hash& hash) const noexcept
    -> bool
{
    Lock lock(lock_);

    return lmdb_.Exists(BlockHeaderDisconnected, hash.Bytes());
}

auto Headers::HaveCheckpoint() const noexcept -> bool
{
    Lock lock(lock_);

    return 0 < checkpoint(lock).first;
}

auto Headers::header_exists(const Lock& lock, const block::Hash& hash)
    const noexcept -> bool
{
    return common_.BlockHeaderExists(hash) &&
           lmdb_.Exists(BlockHeaderMetadata, hash.Bytes());
}

auto Headers::HeaderExists(const block::Hash& hash) const noexcept -> bool
{
    Lock lock(lock_);

    return header_exists(lock, hash);
}

auto Headers::import_genesis(const blockchain::Type type) const noexcept -> void
{
    auto success{false};
    const auto& hash = node::HeaderOracle::GenesisBlockHash(type);

    try {
        const auto serialized = common_.LoadBlockHeader(hash);

        if (!lmdb_.Exists(BlockHeaderMetadata, hash.Bytes())) {
            auto genesis =
                api_.Factory().InternalSession().BlockHeader(serialized);

            OT_ASSERT(genesis);
            block::internal::Header::SerializedType proto{};
            genesis->Internal().Serialize(proto);

            const auto result = lmdb_.Store(
                BlockHeaderMetadata,
                hash.Bytes(),
                std::move(proto::ToString(proto.local())));

            OT_ASSERT(result.first);
        }
    } catch (...) {
        auto genesis = std::unique_ptr<blockchain::block::Header>{
            factory::GenesisBlockHeader(api_, type)};

        OT_ASSERT(genesis);
        OT_ASSERT(hash == genesis->Hash());

        success = common_.StoreBlockHeader(*genesis);

        OT_ASSERT(success);
        block::internal::Header::SerializedType proto{};
        genesis->Internal().Serialize(proto);

        success = lmdb_
                      .Store(
                          BlockHeaderMetadata,
                          hash.Bytes(),
                          proto::ToString(proto.local()))
                      .first;

        OT_ASSERT(success);
    }

    OT_ASSERT(HeaderExists(hash));

    if (0 > best().first) {
        auto transaction = lmdb_.TransactionRW();
        success = push_best({0, hash}, true, transaction);

        OT_ASSERT(success);

        success = transaction.Finalize(true);

        OT_ASSERT(success);

        const auto best = this->best();

        OT_ASSERT(0 == best.first);
        OT_ASSERT(hash == best.second);
    }

    OT_ASSERT(0 <= best().first);
}

auto Headers::IsSibling(const block::Hash& hash) const noexcept -> bool
{
    Lock lock(lock_);

    return lmdb_.Exists(BlockHeaderSiblings, hash.Bytes());
}

auto Headers::load_bitcoin_header(const block::Hash& hash) const
    -> std::unique_ptr<bitcoin::block::Header>
{
    auto proto = common_.LoadBlockHeader(hash);
    const auto haveMeta =
        lmdb_.Load(BlockHeaderMetadata, hash.Bytes(), [&](const auto data) {
            *proto.mutable_local() =
                proto::Factory<proto::BlockchainBlockLocalData>(
                    data.data(), data.size());
        });

    if (!haveMeta) {
        throw std::out_of_range("Block header metadata not found");
    }

    auto output = factory::BitcoinBlockHeader(api_, proto);

    if (!output) { throw std::out_of_range("Wrong header format"); }

    return output;
}

auto Headers::load_header(const block::Hash& hash) const
    -> std::unique_ptr<block::Header>
{
    auto proto = common_.LoadBlockHeader(hash);
    const auto haveMeta =
        lmdb_.Load(BlockHeaderMetadata, hash.Bytes(), [&](const auto data) {
            *proto.mutable_local() =
                proto::Factory<proto::BlockchainBlockLocalData>(
                    data.data(), data.size());
        });

    if (!haveMeta) {
        throw std::out_of_range("Block header metadata not found");
    }

    auto output = api_.Factory().InternalSession().BlockHeader(proto);

    OT_ASSERT(output);

    return output;
}

auto Headers::pop_best(const std::size_t i, MDB_txn* parent) const noexcept
    -> bool
{
    return lmdb_.Delete(BlockHeaderBest, tsv(i), parent);
}

auto Headers::push_best(
    const block::Position& next,
    const bool setTip,
    MDB_txn* parent) const noexcept -> bool
{
    OT_ASSERT(nullptr != parent);

    auto output = lmdb_.Store(
        BlockHeaderBest,
        tsv(static_cast<std::size_t>(next.first)),
        next.second.Bytes(),
        parent);

    if (output.first && setTip) {
        output = lmdb_.Store(
            ChainData,
            tsv(static_cast<std::size_t>(Key::TipHeight)),
            tsv(static_cast<std::size_t>(next.first)),
            parent);
    }

    return output.first;
}

auto Headers::RecentHashes(alloc::Resource* alloc) const noexcept
    -> Vector<block::Hash>
{
    Lock lock(lock_);

    return recent_hashes(lock, alloc);
}

auto Headers::recent_hashes(const Lock& lock, alloc::Resource* alloc)
    const noexcept -> Vector<block::Hash>
{
    auto output = Vector<block::Hash>{alloc};
    lmdb_.Read(
        BlockHeaderBest,
        [&](const auto, const auto value) -> bool {
            output.emplace_back(value);

            return 100 > output.size();
        },
        storage::lmdb::LMDB::Dir::Backward);

    return output;
}

auto Headers::SiblingHashes() const noexcept -> database::Hashes
{
    Lock lock(lock_);
    auto output = database::Hashes{};
    lmdb_.Read(
        BlockHeaderSiblings,
        [&](const auto, const auto value) -> bool {
            output.emplace(value);

            return true;
        },
        storage::lmdb::LMDB::Dir::Forward);

    return output;
}

auto Headers::TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<bitcoin::block::Header>
{
    try {
        return load_bitcoin_header(hash);
    } catch (...) {
        return {};
    }
}

auto Headers::TryLoadHeader(const block::Hash& hash) const noexcept
    -> std::unique_ptr<block::Header>
{
    try {
        return LoadHeader(hash);
    } catch (...) {
        return {};
    }
}
}  // namespace opentxs::blockchain::database
