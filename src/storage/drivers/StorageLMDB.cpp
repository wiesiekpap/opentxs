// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "storage/drivers/StorageLMDB.hpp"  // IWYU pragma: associated

#include <string>
#include <utility>

#include "2_Factory.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "storage/StorageConfig.hpp"
#include "util/LMDB.hpp"

#define OT_METHOD "opentxs::StorageLMDB::"

namespace opentxs
{
auto Factory::StorageLMDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket) -> opentxs::api::storage::Plugin*
{
    return new opentxs::storage::implementation::StorageLMDB(
        storage, config, hash, random, bucket);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageLMDB::StorageLMDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , table_names_({
          {Table::Control, config.lmdb_control_table_},
          {Table::A, config.lmdb_primary_bucket_},
          {Table::B, config.lmdb_secondary_bucket_},
      })
    , lmdb_(
          table_names_,
          config.path_,
          {
              {Table::Control, 0},
              {Table::A, 0},
              {Table::B, 0},
          })
{
    LogVerbose(OT_METHOD)(__func__)(": Using ")(config_.path_).Flush();
    Init_StorageLMDB();
}

void StorageLMDB::Cleanup() { Cleanup_StorageLMDB(); }

void StorageLMDB::Cleanup_StorageLMDB() {}

auto StorageLMDB::EmptyBucket(const bool bucket) const -> bool
{
    return lmdb_.Delete(get_table(bucket));
}

auto StorageLMDB::get_table(const bool bucket) const -> StorageLMDB::Table
{
    return (bucket) ? Table::A : Table::B;
}

void StorageLMDB::Init_StorageLMDB()
{
    LogVerbose(OT_METHOD)(__func__)(": Database initialized.").Flush();
}

auto StorageLMDB::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const -> bool
{
    value = {};
    lmdb_.Load(
        get_table(bucket), key, [&](const auto data) -> void { value = data; });

    return false == value.empty();
}

auto StorageLMDB::LoadRoot() const -> std::string
{
    auto output = std::string{};
    lmdb_.Load(
        Table::Control, config_.lmdb_root_key_, [&](const auto data) -> void {
            output = data;
        });

    return output;
}

void StorageLMDB::store(
    const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    if (isTransaction) {
        lmdb_.Queue(get_table(bucket), key, value);
        promise->set_value(true);
    } else {
        const auto output = lmdb_.Store(get_table(bucket), key, value);
        promise->set_value(output.first);
    }
}

auto StorageLMDB::StoreRoot(const bool commit, const std::string& hash) const
    -> bool
{
    if (commit) {
        if (lmdb_.Queue(Table::Control, config_.lmdb_root_key_, hash)) {

            return lmdb_.Commit();
        }

        return false;
    } else {

        return lmdb_.Store(Table::Control, config_.lmdb_root_key_, hash).first;
    }
}

StorageLMDB::~StorageLMDB() { Cleanup_StorageLMDB(); }
}  // namespace opentxs::storage::implementation
