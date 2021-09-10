// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "storage/drivers/StorageSqlite3.hpp"  // IWYU pragma: associated

#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

#include "2_Factory.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "storage/StorageConfig.hpp"

#define OT_METHOD "opentxs::StorageSqlite3::"

namespace opentxs
{
auto Factory::StorageSqlite3(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket) -> opentxs::api::storage::Plugin*
{
    return new opentxs::storage::implementation::StorageSqlite3(
        storage, config, hash, random, bucket);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageSqlite3::StorageSqlite3(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , folder_(config.path_)
    , transaction_lock_()
    , transaction_bucket_(Flag::Factory(false))
    , pending_()
    , db_(nullptr)
{
    Init_StorageSqlite3();
}

auto StorageSqlite3::bind_key(
    const std::string& source,
    const std::string& key,
    const std::size_t start) const -> std::string
{
    OT_ASSERT(std::numeric_limits<int>::max() >= key.size());
    OT_ASSERT(std::numeric_limits<int>::max() >= start);

    sqlite3_stmt* statement{nullptr};
    sqlite3_prepare_v2(db_, source.c_str(), -1, &statement, nullptr);
    sqlite3_bind_text(
        statement,
        static_cast<int>(start),
        key.c_str(),
        static_cast<int>(key.size()),
        SQLITE_STATIC);
    const auto output = expand_sql(statement);
    sqlite3_finalize(statement);

    return output;
}

void StorageSqlite3::Cleanup() { Cleanup_StorageSqlite3(); }

void StorageSqlite3::Cleanup_StorageSqlite3() { sqlite3_close(db_); }

void StorageSqlite3::commit(std::stringstream& sql) const
{
    sql << "COMMIT TRANSACTION;";
}

auto StorageSqlite3::commit_transaction(const std::string& rootHash) const
    -> bool
{
    Lock lock(transaction_lock_);
    std::stringstream sql{};
    start_transaction(sql);
    set_data(sql);
    set_root(rootHash, sql);
    commit(sql);
    pending_.clear();
    LogVerbose(OT_METHOD)(__func__)(sql.str()).Flush();

    return (
        SQLITE_OK ==
        sqlite3_exec(db_, sql.str().c_str(), nullptr, nullptr, nullptr));
}

auto StorageSqlite3::Create(const std::string& tablename) const -> bool
{
    const std::string createTable = "create table if not exists ";
    const std::string tableFormat = " (k text PRIMARY KEY, v BLOB);";
    const std::string sql = createTable + "`" + tablename + "`" + tableFormat;

    return (
        SQLITE_OK == sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr));
}

auto StorageSqlite3::EmptyBucket(const bool bucket) const -> bool
{
    return Purge(GetTableName(bucket));
}

auto StorageSqlite3::expand_sql(sqlite3_stmt* statement) const -> std::string
{
    const auto sql = sqlite3_expanded_sql(statement);
    const std::string output{sql};
    sqlite3_free(sql);

    return output;
}

auto StorageSqlite3::GetTableName(const bool bucket) const -> std::string
{
    return bucket ? config_.sqlite3_secondary_bucket_
                  : config_.sqlite3_primary_bucket_;
}

void StorageSqlite3::Init_StorageSqlite3()
{
    const std::string filename = folder_ + "/" + config_.sqlite3_db_file_;

    if (SQLITE_OK ==
        sqlite3_open_v2(
            filename.c_str(),
            &db_,
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX,
            nullptr)) {
        sqlite3_exec(
            db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
        Create(config_.sqlite3_primary_bucket_);
        Create(config_.sqlite3_secondary_bucket_);
        Create(config_.sqlite3_control_table_);
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed to initialize database.")
            .Flush();

        OT_FAIL
    }
}

auto StorageSqlite3::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const -> bool
{
    return Select(key, GetTableName(bucket), value);
}

auto StorageSqlite3::LoadRoot() const -> std::string
{
    std::string value{""};

    if (Select(
            config_.sqlite3_root_key_, config_.sqlite3_control_table_, value)) {

        return value;
    }

    return "";
}

auto StorageSqlite3::Purge(const std::string& tablename) const -> bool
{
    const std::string sql = "DROP TABLE `" + tablename + "`;";

    if (SQLITE_OK ==
        sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, nullptr)) {
        return Create(tablename);
    }

    return false;
}

auto StorageSqlite3::Select(
    const std::string& key,
    const std::string& tablename,
    std::string& value) const -> bool
{
    sqlite3_stmt* statement{nullptr};
    const std::string query =
        "SELECT v FROM '" + tablename + "' WHERE k GLOB ?1;";
    const auto sql = bind_key(query, key, 1);
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &statement, nullptr);
    LogVerbose(OT_METHOD)(__func__)(sql).Flush();
    auto result = sqlite3_step(statement);
    bool success = false;
    std::size_t retry{3};

    while (0 < retry) {
        switch (result) {
            case SQLITE_DONE:
            case SQLITE_ROW: {
                retry = 0;
                const auto size = sqlite3_column_bytes(statement, 0);
                success = (0 < size);

                if (success) {
                    const auto pResult = sqlite3_column_blob(statement, 0);
                    value.assign(static_cast<const char*>(pResult), size);
                }
            } break;
            case SQLITE_BUSY: {
                LogOutput(OT_METHOD)(__func__)(": Busy.").Flush();
                result = sqlite3_step(statement);
                --retry;
            } break;
            default: {
                LogOutput(OT_METHOD)(__func__)(": Unknown error (")(
                    result)(").")
                    .Flush();
                result = sqlite3_step(statement);
                --retry;
            }
        }
    }

    sqlite3_finalize(statement);

    return success;
}

void StorageSqlite3::set_data(std::stringstream& sql) const
{
    OT_ASSERT(std::numeric_limits<int>::max() >= pending_.size());

    sqlite3_stmt* data{nullptr};
    std::stringstream dataSQL{};
    const std::string tablename{GetTableName(transaction_bucket_.get())};
    dataSQL << "INSERT OR REPLACE INTO '" << tablename << "' (k, v) VALUES ";
    auto counter{0};
    const auto size = static_cast<int>(pending_.size());

    for (auto i{0}; i < size; ++i) {
        dataSQL << "(?" << ++counter << ", ?";
        dataSQL << ++counter << ")";

        if (counter < (2 * size)) {
            dataSQL << ", ";
        } else {
            dataSQL << "; ";
        }
    }

    sqlite3_prepare_v2(db_, dataSQL.str().c_str(), -1, &data, nullptr);
    counter = 0;

    for (const auto& it : pending_) {
        const auto& key = it.first;
        const auto& value = it.second;

        OT_ASSERT(std::numeric_limits<int>::max() >= key.size());
        OT_ASSERT(std::numeric_limits<int>::max() >= value.size());

        auto bound = sqlite3_bind_text(
            data,
            ++counter,
            key.c_str(),
            static_cast<int>(key.size()),
            SQLITE_STATIC);

        OT_ASSERT(SQLITE_OK == bound);

        bound = sqlite3_bind_blob(
            data,
            ++counter,
            value.c_str(),
            static_cast<int>(value.size()),
            SQLITE_STATIC);

        OT_ASSERT(SQLITE_OK == bound);
    }

    sql << expand_sql(data) << " ";
    sqlite3_finalize(data);
}

void StorageSqlite3::set_root(
    const std::string& rootHash,
    std::stringstream& sql) const
{
    OT_ASSERT(
        std::numeric_limits<int>::max() >= config_.sqlite3_root_key_.size());
    OT_ASSERT(std::numeric_limits<int>::max() >= rootHash.size());

    sqlite3_stmt* root{nullptr};
    std::stringstream rootSQL{};
    rootSQL << "INSERT OR REPLACE INTO '" << config_.sqlite3_control_table_
            << "'  (k, v) VALUES (?1, ?2); ";
    sqlite3_prepare_v2(db_, rootSQL.str().c_str(), -1, &root, nullptr);
    auto bound = sqlite3_bind_text(
        root,
        1,
        config_.sqlite3_root_key_.c_str(),
        static_cast<int>(config_.sqlite3_root_key_.size()),
        SQLITE_STATIC);

    OT_ASSERT(SQLITE_OK == bound)

    bound = sqlite3_bind_blob(
        root,
        2,
        rootHash.c_str(),
        static_cast<int>(rootHash.size()),
        SQLITE_STATIC);

    OT_ASSERT(SQLITE_OK == bound)

    sql << expand_sql(root) << " ";
    sqlite3_finalize(root);
}

void StorageSqlite3::start_transaction(std::stringstream& sql) const
{
    sql << "BEGIN TRANSACTION; ";
}

void StorageSqlite3::store(
    const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (isTransaction) {
        Lock lock(transaction_lock_);
        transaction_bucket_->Set(bucket);
        pending_.emplace_back(key, value);
        promise->set_value(true);
    } else {
        promise->set_value(Upsert(key, GetTableName(bucket), value));
    }
}

auto StorageSqlite3::StoreRoot(const bool commit, const std::string& hash) const
    -> bool
{
    if (commit) {

        return commit_transaction(hash);
    } else {

        return Upsert(
            config_.sqlite3_root_key_, config_.sqlite3_control_table_, hash);
    }
}

auto StorageSqlite3::Upsert(
    const std::string& key,
    const std::string& tablename,
    const std::string& value) const -> bool
{
    OT_ASSERT(std::numeric_limits<int>::max() >= key.size());
    OT_ASSERT(std::numeric_limits<int>::max() >= value.size());

    sqlite3_stmt* statement;
    const std::string query =
        "insert or replace into `" + tablename + "` (k, v) values (?1, ?2);";

    sqlite3_prepare_v2(db_, query.c_str(), -1, &statement, nullptr);
    sqlite3_bind_text(
        statement, 1, key.c_str(), static_cast<int>(key.size()), SQLITE_STATIC);
    sqlite3_bind_blob(
        statement,
        2,
        value.c_str(),
        static_cast<int>(value.size()),
        SQLITE_STATIC);
    LogVerbose(OT_METHOD)(__func__)(expand_sql(statement)).Flush();
    const auto result = sqlite3_step(statement);
    sqlite3_finalize(statement);

    return (result == SQLITE_DONE);
}

StorageSqlite3::~StorageSqlite3() { Cleanup_StorageSqlite3(); }
}  // namespace opentxs::storage::implementation
