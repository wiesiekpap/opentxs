// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/api/storage/Multiplex.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Plugin;
class Storage;
}  // namespace storage
}  // namespace api

namespace storage
{
class Root;
}  // namespace storage

class Factory;
class Flag;
class StorageConfig;
class String;
}  // namespace opentxs

namespace opentxs::storage::implementation
{
class StorageMultiplex final : virtual public opentxs::api::storage::Multiplex
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool final;
    auto Load(const std::string& key, const bool checking, std::string& value)
        const -> bool final;
    auto LoadRoot() const -> std::string final;
    auto Migrate(
        const std::string& key,
        const opentxs::api::storage::Driver& to) const -> bool final;
    auto Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const -> bool final;
    void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const final;
    auto Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const -> bool final;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool final;

    auto BestRoot(bool& primaryOutOfSync) -> std::string final;
    void InitBackup() final;
    void InitEncryptedBackup(crypto::key::Symmetric& key) final;
    auto Primary() -> opentxs::api::storage::Driver& final;
    void SynchronizePlugins(
        const std::string& hash,
        const storage::Root& root,
        const bool syncPrimary) final;

    ~StorageMultiplex() final;

private:
    friend Factory;

    const api::storage::Storage& storage_;
    const Flag& primary_bucket_;
    const StorageConfig& config_;
    std::unique_ptr<opentxs::api::storage::Plugin> primary_plugin_;
    std::vector<std::unique_ptr<opentxs::api::storage::Plugin>> backup_plugins_;
    const Digest digest_;
    const Random random_;
    OTSymmetricKey null_;

    auto Cleanup() -> void;
    auto Cleanup_StorageMultiplex() -> void;
    auto init(
        const std::string& primary,
        std::unique_ptr<opentxs::api::storage::Plugin>& plugin) -> void;
    auto init_fs(std::unique_ptr<opentxs::api::storage::Plugin>& plugin)
        -> void;
    auto init_fs_backup(const std::string& dir) -> void;
    auto init_lmdb(std::unique_ptr<opentxs::api::storage::Plugin>& plugin)
        -> void;
    auto init_memdb(std::unique_ptr<opentxs::api::storage::Plugin>& plugin)
        -> void;
    auto init_sqlite(std::unique_ptr<opentxs::api::storage::Plugin>& plugin)
        -> void;
    auto Init_StorageMultiplex(
        const String& primary,
        const bool migrate,
        const String& previous) -> void;
    auto migrate_primary(const std::string& from, const std::string& to)
        -> void;

    StorageMultiplex(
        const api::storage::Storage& storage,
        const Flag& primaryBucket,
        const StorageConfig& config,
        const String& primary,
        const bool migrate,
        const String& previous,
        const Digest& hash,
        const Random& random);
    StorageMultiplex() = delete;
    StorageMultiplex(const StorageMultiplex&) = delete;
    StorageMultiplex(StorageMultiplex&&) = delete;
    auto operator=(const StorageMultiplex&) -> StorageMultiplex& = delete;
    auto operator=(StorageMultiplex&&) -> StorageMultiplex& = delete;
};
}  // namespace opentxs::storage::implementation
