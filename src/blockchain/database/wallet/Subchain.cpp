// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "blockchain/database/wallet/Subchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <future>
#include <iterator>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "blockchain/database/wallet/Pattern.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/SubchainID.hpp"
#include "internal/api/network/Network.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/LMDB.hpp"

#define OT_METHOD "opentxs::blockchain::database::SubchainData::"

namespace opentxs::blockchain::database::wallet
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

using Mode = storage::lmdb::LMDB::Mode;

constexpr auto config_{Table::Config};
constexpr auto last_indexed_{Table::SubchainLastIndexed};
constexpr auto last_scanned_{Table::SubchainLastScanned};
constexpr auto id_index_{Table::SubchainID};
constexpr auto patterns_{Table::WalletPatterns};
constexpr auto pattern_index_{Table::SubchainPatterns};
constexpr auto match_index_{Table::SubchainMatches};

struct SubchainData::Imp {
    auto GetSubchainID(
        const NodeID& subaccount,
        const Subchain subchain,
        MDB_txn* tx) const noexcept -> pSubchainIndex
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();
        const auto output = subchain_index(
            subaccount, subchain, default_filter_type_, current_version_);
        const auto& s = [&]() -> const db::SubchainID& {
            auto& map = cache_.subchain_id_;
            auto lock = Lock{cache_.id_lock_};

            if (auto it = map.find(output); map.end() != it) {

                return it->second;
            }

            const auto [it, added] = map.try_emplace(
                output,
                subchain,
                default_filter_type_,
                current_version_,
                subaccount);

            OT_ASSERT(added);

            return it->second;
        }();
        const auto key = output->Bytes();

        if (false == lmdb_.Exists(id_index_, key)) {
            const auto rc =
                lmdb_.Store(id_index_, key, reader(s.data_), tx).first;

            OT_ASSERT(rc);
        }

        return output;
    }
    auto GetPatterns(const SubchainIndex& subchain) const noexcept -> Patterns
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        try {
            const auto& key = decode_key(subchain);
            const auto patterns = get_patterns(subchain);

            return load_patterns(key, patterns);
        } catch (const std::exception& e) {
            LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return {};
        }
    }
    auto GetUntestedPatterns(
        const SubchainIndex& subchain,
        const ReadView blockID) const noexcept -> Patterns
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        try {
            const auto& key = decode_key(subchain);
            const auto patterns = [&] {
                const auto all = [&] {
                    auto out = get_patterns(subchain);
                    std::sort(out.begin(), out.end());

                    return out;
                }();
                const auto matches = [&] {
                    auto out = get_matches(blockID);
                    std::sort(out.begin(), out.end());

                    return out;
                }();
                auto out = std::vector<pPatternID>{};
                std::set_difference(
                    std::begin(all),
                    std::end(all),
                    std::begin(matches),
                    std::end(matches),
                    std::back_inserter(out));

                return out;
            }();

            return load_patterns(key, patterns);
        } catch (const std::exception& e) {
            LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return {};
        }
    }
    auto Reorg(
        MDB_txn* tx,
        const node::HeaderOracle& headers,
        const SubchainIndex& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();
        const auto [height, hash] = last_scanned(lock, subchain);

        if (height < lastGoodHeight) {

            return true;
        } else if (height > lastGoodHeight) {

            return set_last_scanned(
                lock, tx, subchain, headers.GetPosition(lastGoodHeight));
        } else {

            return set_last_scanned(
                lock, tx, subchain, headers.GetPosition(lastGoodHeight - 1));
        }
    }
    auto SubchainAddElements(
        const SubchainIndex& subchain,
        const ElementMap& elements) const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();

        try {
            auto output{false};
            const auto key = subchain.Bytes();
            auto newIndices = std::vector<pPatternID>{};
            auto highest = Bip32Index{};
            auto tx = lmdb_.TransactionRW();

            for (const auto& [index, patterns] : elements) {
                const auto id = pattern_id(subchain, index);
                newIndices.emplace_back(id);
                highest = std::max(highest, index);

                for (const auto& pattern : patterns) {
                    const auto data = db::Pattern{index, reader(pattern)};
                    output =
                        lmdb_
                            .Store(
                                patterns_, id->Bytes(), reader(data.data_), tx)
                            .first;

                    if (false == output) {
                        throw std::runtime_error{"failed to store pattern"};
                    }
                }
            }

            output = lmdb_.Store(last_indexed_, key, tsv(highest), tx).first;

            if (false == output) {
                throw std::runtime_error{"failed to update highest indexed"};
            }

            cache_.last_indexed_[subchain] = highest;

            for (auto& patternID : newIndices) {
                output =
                    lmdb_.Store(pattern_index_, key, patternID->Bytes(), tx)
                        .first;

                if (false == output) {
                    throw std::runtime_error{
                        "failed to store subchain pattern index"};
                }
            }

            output = tx.Finalize(true);

            if (false == output) {
                throw std::runtime_error{"failed to commit transaction"};
            }

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return false;
        }
    }
    auto SubchainLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();
        auto& map = cache_.last_indexed_;
        auto cLock = Lock{cache_.index_lock_};
        auto it = map.find(subchain);

        if (map.end() != it) { return it->second; }

        lmdb_.Load(last_indexed_, subchain.Bytes(), [&](const auto bytes) {
            if (sizeof(Bip32Index) == bytes.size()) {
                auto value = Bip32Index{};
                std::memcpy(&value, bytes.data(), bytes.size());
                auto [i, added] = map.try_emplace(subchain, value);

                OT_ASSERT(added);

                it = i;
            } else {

                OT_FAIL;
            }
        });

        if (map.end() != it) { return it->second; }

        return std::nullopt;
    }
    auto SubchainLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position
    {
        auto lock = sLock{lock_};
        upgrade_future_.get();

        return last_scanned(lock, subchain);
    }
    auto SubchainMatchBlock(
        const SubchainIndex& subchain,
        const std::vector<std::pair<ReadView, MatchingIndices>>& results)
        const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();
        auto tx = lmdb_.TransactionRW();
        auto output{true};

        try {
            for (const auto& [blockID, indices] : results) {
                for (const auto& index : indices) {
                    const auto id = pattern_id(subchain, index);
                    output = lmdb_.Store(match_index_, blockID, id->Bytes(), tx)
                                 .first;

                    if (false == output) {
                        throw std::runtime_error{"Failed to add match"};
                    }
                }
            }

            output = tx.Finalize(true);

            if (false == output) {
                throw std::runtime_error{"failed to commit transaction"};
            }

            return output;
        } catch (const std::exception& e) {
            LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return false;
        }
    }
    auto SubchainSetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool
    {
        auto lock = eLock{lock_};
        upgrade_future_.get();

        return set_last_scanned(lock, nullptr, subchain, position);
    }

    Imp(const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::filter::Type filter) noexcept
        : api_(api)
        , lmdb_(lmdb)
        , default_filter_type_(filter)
        , current_version_([&] {
            auto version = std::optional<VersionNumber>{};
            lmdb_.Load(
                config_, tsv(database::Key::Version), [&](const auto bytes) {
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
        , cache_()
    {
        api_.Network().Asio().Internal().Post(
            ThreadPool::General, [this] { upgrade(); });
    }

private:
    using PatternID = Identifier;
    using pPatternID = OTIdentifier;

    struct Cache {
        // NOTE if an exclusive lock is being held in the parent then locking
        // these mutexes is redundant
        std::mutex id_lock_{};
        std::map<pSubchainIndex, db::SubchainID> subchain_id_{};
        std::mutex index_lock_{};
        std::map<pSubchainIndex, Bip32Index> last_indexed_{};
        std::mutex scanned_lock_{};
        std::map<pSubchainIndex, db::Position> last_scanned_{};
    };

    const api::Session& api_;
    const storage::lmdb::LMDB& lmdb_;
    const filter::Type default_filter_type_;
    const VersionNumber current_version_;
    std::promise<void> upgrade_promise_;
    std::shared_future<void> upgrade_future_;
    mutable std::shared_mutex lock_;
    mutable Cache cache_;

    auto decode_key(const SubchainIndex& key) const noexcept(false)
        -> const db::SubchainID&
    {
        auto& map = cache_.subchain_id_;
        auto lock = Lock{cache_.id_lock_};
        auto it = map.find(key);

        if (map.end() != it) { return it->second; }

        lmdb_.Load(id_index_, key.Bytes(), [&](const auto bytes) {
            try {
                auto [first, second] = map.try_emplace(key, bytes);
                it = first;
            } catch (const std::exception& e) {
                LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();
            }
        });

        if (map.end() == it) {
            const auto error =
                std::string{"key "} + key.str() + " invalid or not found";
            throw std::runtime_error{error};
        }

        return it->second;
    }
    auto get_matches(const ReadView block) const noexcept
        -> std::vector<pPatternID>
    {
        auto out = std::vector<pPatternID>{};
        lmdb_.Load(
            match_index_,
            block,
            [&](const auto bytes) {
                if (0u < bytes.size()) {
                    auto& id = out.emplace_back(api_.Factory().Identifier());
                    id->Assign(bytes.data(), bytes.size());
                } else {

                    OT_FAIL;
                }
            },
            Mode::Multiple);

        return out;
    }
    auto get_patterns(const SubchainIndex& subchain) const noexcept
        -> std::vector<pPatternID>
    {
        auto out = std::vector<pPatternID>{};
        lmdb_.Load(
            pattern_index_,
            subchain.Bytes(),
            [&](const auto bytes) {
                if (0u < bytes.size()) {
                    auto& id = out.emplace_back(api_.Factory().Identifier());
                    id->Assign(bytes.data(), bytes.size());
                } else {

                    OT_FAIL;
                }
            },
            Mode::Multiple);

        return out;
    }
    template <typename LockType>
    auto last_scanned(const LockType& lock, const SubchainIndex& subchain)
        const noexcept -> block::Position
    {
        auto& map = cache_.last_scanned_;
        auto cLock = Lock{cache_.scanned_lock_};
        auto it = map.find(subchain);

        if (map.end() != it) { return it->second.Decode(api_); }

        lmdb_.Load(last_scanned_, subchain.Bytes(), [&](const auto bytes) {
            try {
                auto [first, second] = map.try_emplace(subchain, bytes);
                it = first;
            } catch (const std::exception& e) {
                LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();
            }
        });

        if (map.end() == it) {
            static const auto null = make_blank<block::Position>::value(api_);

            return null;
        }

        return it->second.Decode(api_);
    }

    auto load_patterns(
        const db::SubchainID& key,
        const std::vector<pPatternID>& patterns) const noexcept(false)
        -> Patterns
    {
        auto output = Patterns{};
        const auto& subaccount = key.SubaccountID(api_);
        const auto subchain = key.Type();

        for (const auto& id : patterns) {
            lmdb_.Load(
                patterns_,
                id->Bytes(),
                [&](const auto bytes) {
                    const auto data = db::Pattern{bytes};
                    output.emplace_back(Parent::Pattern{
                        {data.Index(), {subchain, subaccount}},
                        space(data.Data())});
                },
                Mode::Multiple);
        }

        return output;
    }
    auto pattern_id(const SubchainIndex& subchain, const Bip32Index index)
        const noexcept -> pPatternID
    {
        auto preimage = OTData{subchain};
        preimage->Concatenate(&index, sizeof(index));
        auto output = api_.Factory().Identifier();
        output->CalculateDigest(preimage->Bytes());

        return output;
    }
    auto set_last_scanned(
        const eLock& lock,
        MDB_txn* tx,
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool
    {
        try {
            const auto key = subchain.Bytes();
            const auto s = db::Position{position};

            if (!lmdb_.Store(last_scanned_, key, reader(s.data_), tx).first) {
                throw std::runtime_error{"failed to update database"};
            }

            auto& map = cache_.last_scanned_;
            map.erase(subchain);
            const auto [it, added] = map.try_emplace(subchain, position);

            OT_ASSERT(added);

            return true;
        } catch (const std::exception& e) {
            LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return false;
        }
    }
    auto subchain_index(
        const NodeID& subaccount,
        const Subchain subchain,
        const filter::Type type,
        const VersionNumber version) const noexcept -> pSubchainIndex
    {
        auto preimage = OTData{subaccount};
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

    Imp() = delete;
    Imp(const Imp&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

SubchainData::SubchainData(
    const api::Session& api,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::filter::Type filter) noexcept
    : imp_(std::make_unique<Imp>(api, lmdb, filter))
{
    OT_ASSERT(imp_);
}

auto SubchainData::GetSubchainID(
    const NodeID& subaccount,
    const Subchain subchain,
    MDB_txn* tx) const noexcept -> pSubchainIndex
{
    return imp_->GetSubchainID(subaccount, subchain, tx);
}

auto SubchainData::GetPatterns(const SubchainIndex& subchain) const noexcept
    -> Patterns
{
    return imp_->GetPatterns(subchain);
}

auto SubchainData::GetUntestedPatterns(
    const SubchainIndex& subchain,
    const ReadView blockID) const noexcept -> Patterns
{
    return imp_->GetUntestedPatterns(subchain, blockID);
}

auto SubchainData::Reorg(
    MDB_txn* tx,
    const node::HeaderOracle& headers,
    const SubchainIndex& subchain,
    const block::Height lastGoodHeight) const noexcept(false) -> bool
{
    return imp_->Reorg(tx, headers, subchain, lastGoodHeight);
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

auto SubchainData::SubchainMatchBlock(
    const SubchainIndex& index,
    const std::vector<std::pair<ReadView, MatchingIndices>>& results)
    const noexcept -> bool
{
    return imp_->SubchainMatchBlock(index, results);
}

auto SubchainData::SubchainSetLastScanned(
    const SubchainIndex& subchain,
    const block::Position& position) const noexcept -> bool
{
    return imp_->SubchainSetLastScanned(subchain, position);
}

SubchainData::~SubchainData() = default;
}  // namespace opentxs::blockchain::database::wallet
