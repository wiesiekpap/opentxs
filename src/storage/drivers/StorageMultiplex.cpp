// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "storage/drivers/StorageMultiplex.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include "2_Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Plugin.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "storage/StorageConfig.hpp"
#include "storage/tree/Root.hpp"
#include "storage/tree/Tree.hpp"

#define OT_METHOD "opentxs::storage::implementation::StorageMultiplex::"

namespace opentxs
{
auto Factory::StorageMultiplex(
    const api::storage::Storage& storage,
    const Flag& primaryBucket,
    const StorageConfig& config,
    const String& primary,
    const bool migrate,
    const String& previous,
    const Digest& hash,
    const Random& random) -> opentxs::api::storage::Multiplex*
{
    return new opentxs::storage::implementation::StorageMultiplex(
        storage,
        primaryBucket,
        config,
        primary,
        migrate,
        previous,
        hash,
        random);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageMultiplex::StorageMultiplex(
    const api::storage::Storage& storage,
    const Flag& primaryBucket,
    const StorageConfig& config,
    const String& primary,
    const bool migrate,
    const String& previous,
    const Digest& hash,
    const Random& random)
    : storage_(storage)
    , primary_bucket_(primaryBucket)
    , config_(config)
    , primary_plugin_()
    , backup_plugins_()
    , digest_(hash)
    , random_(random)
    , null_(crypto::key::Symmetric::Factory())
{
    Init_StorageMultiplex(primary, migrate, previous);
}

auto StorageMultiplex::BestRoot(bool& primaryOutOfSync) -> std::string
{
    OT_ASSERT(primary_plugin_);

    const std::string originalHash = primary_plugin_->LoadRoot();
    std::shared_ptr<storage::Root> bestRoot{nullptr};
    std::shared_ptr<storage::Root> localRoot{nullptr};
    std::string bestHash{originalHash};
    std::uint64_t bestVersion{0};
    auto bucket = Flag::Factory(false);

    try {
        localRoot.reset(new storage::Root(
            *this, bestHash, std::numeric_limits<std::int64_t>::max(), bucket));
        bestVersion = localRoot->Sequence();
        bestRoot = localRoot;
    } catch (std::runtime_error&) {
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        std::string rootHash = plugin->LoadRoot();
        std::uint64_t localVersion{0};

        try {
            localRoot.reset(new storage::Root(
                *this,
                rootHash,
                std::numeric_limits<std::int64_t>::max(),
                bucket));
            localVersion = localRoot->Sequence();
        } catch (std::runtime_error&) {
        }

        if (localVersion > bestVersion) {
            bestVersion = localVersion;
            bestHash = rootHash;
            bestRoot = localRoot;
        }
    }

    if (0 == bestVersion) { bestHash = ""; }

    if (originalHash != bestHash) {
        primary_plugin_->StoreRoot(false, bestHash);
        bestRoot->Save(*primary_plugin_);
        primaryOutOfSync = true;
    } else {
        primaryOutOfSync = false;
    }

    return bestHash;
}

void StorageMultiplex::Cleanup() { Cleanup_StorageMultiplex(); }

void StorageMultiplex::Cleanup_StorageMultiplex() {}

auto StorageMultiplex::EmptyBucket(const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        plugin->EmptyBucket(bucket);
    }

    return primary_plugin_->EmptyBucket(bucket);
}

void StorageMultiplex::init(
    const std::string& primary,
    std::unique_ptr<opentxs::api::storage::Plugin>& plugin)
{
    if (OT_STORAGE_PRIMARY_PLUGIN_MEMDB == primary) {
        init_memdb(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_LMDB == primary) {
        init_lmdb(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_SQLITE == primary) {
        init_sqlite(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_FS == primary) {
        init_fs(plugin);
    }

    OT_ASSERT(plugin);
}

void StorageMultiplex::init_memdb(
    std::unique_ptr<opentxs::api::storage::Plugin>& plugin)
{
    LogVerbose(OT_METHOD)(__func__)(": Initializing primary MemDB plugin.")
        .Flush();
    plugin.reset(Factory::StorageMemDB(
        storage_, config_, digest_, random_, primary_bucket_));
}

void StorageMultiplex::Init_StorageMultiplex(
    const String& primary,
    const bool migrate,
    const String& previous)
{
    if (migrate) {
        migrate_primary(previous.Get(), primary.Get());
    } else {
        init(primary.Get(), primary_plugin_);
    }

    OT_ASSERT(primary_plugin_);
}

void StorageMultiplex::InitBackup()
{
    if (config_.fs_backup_directory_.empty()) { return; }

    init_fs_backup(config_.fs_backup_directory_);
}

void StorageMultiplex::InitEncryptedBackup(
    [[maybe_unused]] crypto::key::Symmetric& key)
{
    if (config_.fs_encrypted_backup_directory_.empty()) { return; }

    init_fs_backup(config_.fs_encrypted_backup_directory_);
}

auto StorageMultiplex::Load(
    const std::string& key,
    const bool checking,
    std::string& value) const -> bool
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->Load(key, checking, value)) { return true; }

    if (false == checking) {
        LogVerbose(OT_METHOD)(__func__)(
            ": key not found by primary storage plugin.")
            .Flush();
    }

    std::size_t count{0};

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->Load(key, checking, value)) {
            auto notUsed = key;
            primary_plugin_->Store(false, value, notUsed);

            return true;
        }

        if (false == checking) {
            LogVerbose(OT_METHOD)(__func__)(
                ": key not found by backup storage plugin ")
                .Flush();
        }

        ++count;
    }

    if (false == checking) {
        LogOutput(OT_METHOD)(__func__)(": Key not found by any plugin.")
            .Flush();

        throw std::runtime_error("Key not found by any plugin");
    }

    return false;
}

auto StorageMultiplex::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->LoadFromBucket(key, value, bucket)) { return true; }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->LoadFromBucket(key, value, bucket)) { return true; }
    }

    return false;
}

auto StorageMultiplex::LoadRoot() const -> std::string
{
    OT_ASSERT(primary_plugin_);

    std::string root = primary_plugin_->LoadRoot();

    if (false == root.empty()) { return root; }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        root = plugin->LoadRoot();

        if (false == root.empty()) { return root; }
    }

    return root;
}

auto StorageMultiplex::Migrate(
    const std::string& key,
    const opentxs::api::storage::Driver& to) const -> bool
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->Migrate(key, to)) { return true; }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->Migrate(key, to)) { return true; }
    }

    return false;
}

void StorageMultiplex::migrate_primary(
    const std::string& from,
    const std::string& to)
{
    auto& old = primary_plugin_;

    init(from, old);

    OT_ASSERT(old);

    std::unique_ptr<opentxs::api::storage::Plugin> newPlugin{nullptr};
    init(to, newPlugin);

    OT_ASSERT(newPlugin);

    const std::string rootHash = old->LoadRoot();
    std::shared_ptr<storage::Root> root{nullptr};
    auto bucket = Flag::Factory(false);
    root.reset(new storage::Root(
        *this, rootHash, std::numeric_limits<std::int64_t>::max(), bucket));

    OT_ASSERT(root);

    const auto& tree = root->Tree();
    const auto migrated = tree.Migrate(*newPlugin);

    if (migrated) {
        LogOutput(OT_METHOD)(__func__)(
            ": Successfully migrated to new primary plugin.")
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed to migrate primary plugin.")
            .Flush();

        OT_FAIL;
    }

    Lock lock(root->write_lock_);
    root->save(lock, *newPlugin);
    newPlugin->StoreRoot(false, rootHash);
    old.reset(newPlugin.release());
}

auto StorageMultiplex::Primary() -> opentxs::api::storage::Driver&
{
    OT_ASSERT(primary_plugin_);

    return *primary_plugin_;
}

auto StorageMultiplex::Store(
    const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    std::vector<std::promise<bool>> promises{};
    std::vector<std::future<bool>> futures{};
    promises.push_back(std::promise<bool>());
    auto& primaryPromise = promises.back();
    futures.push_back(primaryPromise.get_future());
    primary_plugin_->Store(isTransaction, key, value, bucket, primaryPromise);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        promises.push_back(std::promise<bool>());
        auto& promise = promises.back();
        futures.push_back(promise.get_future());
        plugin->Store(isTransaction, key, value, bucket, promise);
    }

    bool output = false;

    for (auto& future : futures) { output |= future.get(); }

    return output;
}

void StorageMultiplex::Store(
    const bool,
    const std::string&,
    const std::string&,
    const bool,
    std::promise<bool>&) const
{
    // This method should never be called

    OT_FAIL;
}

auto StorageMultiplex::Store(
    const bool isTransaction,
    const std::string& key,
    std::string& value) const -> bool
{
    OT_ASSERT(primary_plugin_);

    bool output = primary_plugin_->Store(isTransaction, key, value);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        output |= plugin->Store(isTransaction, key, value);
    }

    return output;
}

auto StorageMultiplex::StoreRoot(const bool commit, const std::string& hash)
    const -> bool
{
    OT_ASSERT(primary_plugin_);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        plugin->StoreRoot(commit, hash);
    }

    return primary_plugin_->StoreRoot(commit, hash);
}

void StorageMultiplex::SynchronizePlugins(
    const std::string& hash,
    const storage::Root& root,
    const bool syncPrimary)
{
    const auto& tree = root.Tree();

    if (syncPrimary) {
        OT_ASSERT(primary_plugin_);

        LogOutput(OT_METHOD)(__func__)(": Primary plugin is out of sync.")
            .Flush();

        const auto migrated = tree.Migrate(*primary_plugin_);

        if (migrated) {
            LogOutput(OT_METHOD)(__func__)(
                ": Successfully restored primary plugin from backup.")
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to restore primary plugin from backup.")
                .Flush();
        }
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (hash == plugin->LoadRoot()) { continue; }

        LogOutput(OT_METHOD)(__func__)(
            ": Backup plugin is uninitialized or out of sync.")
            .Flush();

        if (tree.Migrate(*plugin)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Successfully initialized backup plugin.")
                .Flush();
        } else {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to initialize backup plugin.")
                .Flush();
        }

        if (false == root.Save(*plugin)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to update root index object for backup plugin.")
                .Flush();
        }

        if (false == plugin->StoreRoot(false, hash)) {
            LogOutput(OT_METHOD)(__func__)(
                ": Failed to update root hash for backup plugin.")
                .Flush();
        }
    }
}

StorageMultiplex::~StorageMultiplex() { Cleanup_StorageMultiplex(); }
}  // namespace opentxs::storage::implementation
