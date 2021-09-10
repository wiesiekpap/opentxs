// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "storage/tree/Notary.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/protobuf/BlindedSeriesList.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/SpentTokenList.pb.h"
#include "opentxs/protobuf/StorageEnums.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"
#include "opentxs/protobuf/StorageNotary.pb.h"
#include "opentxs/protobuf/verify/SpentTokenList.hpp"
#include "opentxs/protobuf/verify/StorageNotary.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

#define STORAGE_NOTARY_VERSION 1
#if OT_CASH
#define STORAGE_MINT_SERIES_VERSION 1
#define STORAGE_MINT_SERIES_HASH_VERSION 2
#define STORAGE_MINT_SPENT_LIST_VERSION 1
#endif
#define OT_METHOD "opentxs::storage::Notary::"

namespace opentxs::storage
{
Notary::Notary(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash,
    const std::string& id)
    : Node(storage, hash)
    , id_(id)
#if OT_CASH
    , mint_map_()
#endif
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(STORAGE_NOTARY_VERSION);
    }
}

#if OT_CASH
auto Notary::CheckSpent(
    const identifier::UnitDefinition& unit,
    const MintSeries series,
    const std::string& key) const -> bool
{
    if (key.empty()) { throw std::runtime_error("Invalid token key"); }

    Lock lock(write_lock_);
    const auto list = get_or_create_list(lock, unit.str(), series);

    for (const auto& spent : list.spent()) {
        if (spent == key) {
            LogTrace(OT_METHOD)(__func__)("Token ")(key)(" is already spent.")
                .Flush();

            return true;
        }
    }

    LogTrace(OT_METHOD)(__func__)("Token ")(key)(" has never been spent.")
        .Flush();

    return false;
}

auto Notary::create_list(
    const std::string& unitID,
    const MintSeries series,
    std::shared_ptr<proto::SpentTokenList>& output) const -> std::string
{
    std::string hash{};
    output.reset(new proto::SpentTokenList);

    OT_ASSERT(output);

    auto& list = *output;
    list.set_version(STORAGE_MINT_SPENT_LIST_VERSION);
    list.set_notary(id_);
    list.set_unit(unitID);
    list.set_series(series);

    const auto saved = driver_.StoreProto(list, hash);

    if (false == saved) {
        throw std::runtime_error("Failed to create spent token list");
    }

    return hash;
}

auto Notary::get_or_create_list(
    const Lock& lock,
    const std::string& unitID,
    const MintSeries series) const -> proto::SpentTokenList
{
    OT_ASSERT(verify_write_lock(lock));

    std::shared_ptr<proto::SpentTokenList> output{};
    auto& hash = mint_map_[unitID][series];

    if (hash.empty()) {
        hash = create_list(unitID, series, output);
    } else {
        driver_.LoadProto(hash, output);
    }

    if (false == bool(output)) {
        throw std::runtime_error("Failed to load spent token list");
    }

    return *output;
}
#endif

void Notary::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNotary> serialized;
    driver_.LoadProto(hash, serialized);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load index file").Flush();

        OT_FAIL;
    }

    init_version(STORAGE_NOTARY_VERSION, *serialized);
    id_ = serialized->id();

#if OT_CASH
    for (const auto& it : serialized->series()) {
        auto& unitMap = mint_map_[it.unit()];

        for (const auto& storageHash : it.series()) {
            const auto series = std::stoul(storageHash.alias());
            unitMap[series] = storageHash.hash();
        }
    }
#endif
}

#if OT_CASH
auto Notary::MarkSpent(
    const identifier::UnitDefinition& unit,
    const MintSeries series,
    const std::string& key) -> bool
{
    if (key.empty()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid key ").Flush();

        return false;
    }

    Lock lock(write_lock_);
    auto list = get_or_create_list(lock, unit.str(), series);
    list.add_spent(key);

    OT_ASSERT(proto::Validate(list, VERBOSE));

    auto& hash = mint_map_[unit.str()][series];
    LogTrace(OT_METHOD)(__func__)(": Token ")(key)(" marked as spent.").Flush();

    return driver_.StoreProto(list, hash);
}
#endif

auto Notary::save(const Lock& lock) const -> bool
{
    if (false == verify_write_lock(lock)) {
        LogOutput(OT_METHOD)(__func__)(": Lock failure").Flush();

        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Notary::serialize() const -> proto::StorageNotary
{
    proto::StorageNotary serialized;
    serialized.set_version(version_);
    serialized.set_id(id_);

#if OT_CASH
    for (const auto& [unitID, seriesMap] : mint_map_) {
        auto& series = *serialized.add_series();
        series.set_version(STORAGE_MINT_SERIES_VERSION);
        series.set_notary(id_);
        series.set_unit(unitID);

        for (const auto& [seriesNumber, hash] : seriesMap) {
            auto& storageHash = *series.add_series();
            const auto seriesString = std::to_string(seriesNumber);
            storageHash.set_version(STORAGE_MINT_SERIES_HASH_VERSION);
            storageHash.set_itemid(Identifier::Factory(seriesString)->str());
            storageHash.set_hash(hash);
            storageHash.set_alias(seriesString);
            storageHash.set_type(proto::STORAGEHASH_PROTO);
        }
    }
#endif

    return serialized;
}
}  // namespace opentxs::storage
