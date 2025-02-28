// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Subchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <future>
#include <shared_mutex>
#include <stdexcept>
#include <type_traits>

#include "blockchain/database/wallet/SubchainCache.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "blockchain/database/wallet/Types.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Thread.hpp"

namespace opentxs::blockchain::database::wallet
{
struct SubchainData::Imp {
    auto GetSubchainID(
        const NodeID& subaccount,
        const crypto::Subchain subchain,
        MDB_txn* tx) const noexcept -> pSubchainIndex
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return cache_.GetIndex(
            subaccount, subchain, default_filter_type_, current_version_, tx);
    }
    auto GetPatterns(const SubchainIndex& id, alloc::Resource* alloc)
        const noexcept -> Patterns
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        try {
            const auto& key = cache_.DecodeIndex(id);

            return load_patterns(lock, key, cache_.GetPatternIndex(id), alloc);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }
    auto Reorg(
        const Lock& headerOracleLock,
        MDB_txn* tx,
        const node::HeaderOracle& headers,
        const SubchainIndex& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool
    {
        auto lock = eLock{lock_};
        const auto post = ScopeGuard{[&] { cache_.Clear(); }};
        upgrade_future_.get();
        const auto [height, hash] = cache_.GetLastScanned(subchain);
        auto target = block::Height{};

        if (height < lastGoodHeight) {
            LogTrace()(OT_PRETTY_CLASS())(
                "no action required for this subchain since last scanned "
                "height of ")(height)(" is below the reorg parent height ")(
                lastGoodHeight)
                .Flush();

            return true;
        } else if (height > lastGoodHeight) {
            target = lastGoodHeight;
        } else {
            target = std::max<block::Height>(lastGoodHeight - 1, 0);
        }

        const auto position =
            headers.Internal().GetPosition(headerOracleLock, target);
        LogTrace()(OT_PRETTY_CLASS())("resetting last scanned to ")(position)
            .Flush();

        if (!cache_.SetLastScanned(subchain, position, tx)) {
            throw std::runtime_error{"database error"};
        }

        return false;
    }
    auto SubchainAddElements(
        const SubchainIndex& subchain,
        const ElementMap& elements) const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();

        try {
            auto output{false};
            auto newIndices = UnallocatedVector<pPatternID>{};
            auto highest = Bip32Index{};
            auto tx = lmdb_.TransactionRW();

            for (const auto& [index, patterns] : elements) {
                const auto id = pattern_id(subchain, index);
                newIndices.emplace_back(id);
                highest = std::max(highest, index);

                for (const auto& pattern : patterns) {
                    output = cache_.AddPattern(id, index, reader(pattern), tx);

                    if (!output) {
                        throw std::runtime_error{"failed to store pattern"};
                    }
                }
            }

            output = cache_.SetLastIndexed(subchain, highest, tx);

            if (!output) {
                throw std::runtime_error{"failed to update highest indexed"};
            }

            for (auto& patternID : newIndices) {
                output = cache_.AddPatternIndex(subchain, patternID, tx);

                if (!output) {
                    throw std::runtime_error{
                        "failed to store subchain pattern index"};
                }
            }

            output = tx.Finalize(true);

            if (!output) {
                throw std::runtime_error{"failed to commit transaction"};
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
            cache_.Clear();

            return false;
        }
    }
    auto SubchainLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return cache_.GetLastIndexed(subchain);
    }
    auto SubchainLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return cache_.GetLastScanned(subchain);
    }
    auto SubchainSetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();

        if (cache_.SetLastScanned(subchain, position, nullptr)) {

            return true;
        } else {
            cache_.Clear();

            return false;
        }
    }

    Imp(const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::cfilter::Type filter) noexcept
        : api_(api)
        , lmdb_(lmdb)
        , default_filter_type_(filter)
        , current_version_([&] {
            auto version = std::optional<VersionNumber>{};
            lmdb_.Load(
                subchain_config_,
                tsv(database::Key::Version),
                [&](const auto bytes) {
                    if (sizeof(VersionNumber) == bytes.size()) {
                        auto& out = version.emplace();
                        std::memcpy(&out, bytes.data(), bytes.size());
                    } else if (sizeof(std::size_t) == bytes.size()) {
                        auto& out = version.emplace();
                        std::memcpy(
                            &out,
                            bytes.data(),
                            std::min(bytes.size(), sizeof(out)));
                    } else {

                        OT_FAIL;
                    }
                });

            OT_ASSERT(version.has_value());

            return version.value();
        }())
        , upgrade_promise_()
        , upgrade_future_(upgrade_promise_.get_future())
        , lock_()
        , cache_(api_, lmdb_)
    {
        api_.Network().Asio().Internal().Post(
            ThreadPool::General, [this] { upgrade(); }, subchainThreadName);
    }
    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

private:
    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    const cfilter::Type default_filter_type_;
    const VersionNumber current_version_;
    std::promise<void> upgrade_promise_;
    std::shared_future<void> upgrade_future_;
    mutable std::shared_mutex lock_;
    mutable SubchainCache cache_;

    template <typename LockType, typename Container>
    auto load_patterns(
        const LockType& lock,
        const db::SubchainID& key,
        const Container& patterns,
        alloc::Resource* alloc) const noexcept(false) -> Patterns
    {
        auto output = Patterns{alloc};
        const auto& subaccount = key.SubaccountID(api_);
        const auto subchain = key.Type();

        for (const auto& id : patterns) {
            for (const auto& data : cache_.GetPattern(id)) {
                output.emplace_back(Parent::Pattern{
                    {data.Index(), {subchain, subaccount}},
                    space(data.Data(), alloc)});
            }
        }

        return output;
    }
    auto pattern_id(const SubchainIndex& subchain, const Bip32Index index)
        const noexcept -> pPatternID
    {

        auto preimage = api_.Factory().Data();
        preimage->Assign(subchain);
        preimage->Concatenate(&index, sizeof(index));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto subchain_index(
        const NodeID& subaccount,
        const crypto::Subchain subchain,
        const cfilter::Type type,
        const VersionNumber version) const noexcept -> pSubchainIndex
    {
        auto preimage = api_.Factory().Data();
        preimage->Assign(subaccount);
        preimage->Concatenate(&subchain, sizeof(subchain));
        preimage->Concatenate(&type, sizeof(type));
        preimage->Concatenate(&version, sizeof(version));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }

    auto upgrade() noexcept -> void
    {
        // TODO
        // 1. read every value from Table::SubchainID
        // 2. if the filter type or version does not match the current value,
        // reindex everything

        upgrade_promise_.set_value();
    }
};

SubchainData::SubchainData(
    const api::Session& api,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::cfilter::Type filter) noexcept
    : imp_(std::make_unique<Imp>(api, lmdb, filter))
{
    OT_ASSERT(imp_);
}

auto SubchainData::GetSubchainID(
    const NodeID& subaccount,
    const crypto::Subchain subchain,
    MDB_txn* tx) const noexcept -> pSubchainIndex
{
    return imp_->GetSubchainID(subaccount, subchain, tx);
}

auto SubchainData::GetPatterns(
    const SubchainIndex& subchain,
    alloc::Resource* alloc) const noexcept -> Patterns
{
    return imp_->GetPatterns(subchain, alloc);
}

auto SubchainData::Reorg(
    const Lock& headerOracleLock,
    MDB_txn* tx,
    const node::HeaderOracle& headers,
    const SubchainIndex& subchain,
    const block::Height lastGoodHeight) const noexcept(false) -> bool
{
    return imp_->Reorg(headerOracleLock, tx, headers, subchain, lastGoodHeight);
}

auto SubchainData::SubchainAddElements(
    const SubchainIndex& subchain,
    const ElementMap& elements) const noexcept -> bool
{
    return imp_->SubchainAddElements(subchain, elements);
}

auto SubchainData::SubchainLastIndexed(
    const SubchainIndex& subchain) const noexcept -> std::optional<Bip32Index>
{
    return imp_->SubchainLastIndexed(subchain);
}

auto SubchainData::SubchainLastScanned(
    const SubchainIndex& subchain) const noexcept -> block::Position
{
    return imp_->SubchainLastScanned(subchain);
}

auto SubchainData::SubchainSetLastScanned(
    const SubchainIndex& subchain,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SubchainSetLastScanned(subchain, position);
}

SubchainData::~SubchainData() = default;
}  // namespace opentxs::blockchain::database::wallet
