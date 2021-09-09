// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "api/StorageParent.hpp"  // IWYU pragma: associated

#include <utility>

#include "2_Factory.hpp"
#include "opentxs/Pimpl.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#endif  // OT_CRYPTO_WITH_BIP32
#include "opentxs/api/Options.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define STORAGE_CONFIG_KEY "storage"

#define OT_METHOD "opentxs::api::implementation::StorageParent::"

namespace opentxs::api::implementation
{
StorageParent::StorageParent(
    const Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const api::Legacy& legacy,
    const std::string& dataFolder)
    : crypto_(crypto)
    , config_(config)
    , args_(std::move(args))
    , gc_interval_(0)
    , data_folder_(dataFolder)
    , storage_config_()
    , migrate_storage_{false}
    , migrate_from_{String::Factory()}
    , primary_storage_plugin_(get_primary_storage_plugin(
          config_,
          storage_config_,
          args_,
          migrate_storage_,
          migrate_from_))
    , archive_directory_(String::Factory())
    , encrypted_directory_(String::Factory())
    , storage_(opentxs::Factory::Storage(
          running,
          crypto_,
          config_,
          legacy,
          data_folder_,
          primary_storage_plugin_,
          archive_directory_,
          gc_interval_,
          encrypted_directory_,
          storage_config_))
#if OT_CRYPTO_WITH_BIP32
    , storage_encryption_key_(opentxs::crypto::key::Symmetric::Factory())
#endif
{
    OT_ASSERT(storage_);
    OT_ASSERT(false == data_folder_.empty())
}

void StorageParent::init(
    [[maybe_unused]] const api::Factory& factory
#if OT_CRYPTO_WITH_BIP32
    ,
    const api::HDSeed& seeds
#endif  // OT_CRYPTO_WITH_BIP32
)
{
    if (encrypted_directory_->empty()) { return; }

#if OT_CRYPTO_WITH_BIP32
    auto seed = seeds.DefaultSeed();

    if (seed.empty()) {
        LogOutput(OT_METHOD)(__func__)(": No default seed.").Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Default seed is: ")(seed)(".")
            .Flush();
    }

    auto reason = factory.PasswordPrompt("Initializaing storage encryption");
    storage_encryption_key_ = seeds.GetStorageKey(seed, reason);

    if (storage_encryption_key_.get()) {
        LogDetail(OT_METHOD)(__func__)(": Obtained storage key ")(
            storage_encryption_key_->ID(reason))
            .Flush();
    } else {
        LogOutput(OT_METHOD)(__func__)(": Failed to load storage key ")(
            seed)(".")
            .Flush();
    }
#endif

    start();
}

auto StorageParent::get_primary_storage_plugin(
    const api::Settings& config,
    const StorageConfig& storageConfig,
    const Options& args,
    bool& migrate,
    String& previous) -> OTString
{
    const auto hardcoded = String::Factory(storageConfig.primary_plugin_);
    const auto commandLine = String::Factory(args.StoragePrimaryPlugin());
    auto configured = String::Factory();
    bool notUsed{false};
    config.Check_str(
        String::Factory(STORAGE_CONFIG_KEY),
        String::Factory(STORAGE_CONFIG_PRIMARY_PLUGIN_KEY),
        configured,
        notUsed);
    const auto haveConfigured = configured->Exists();
    const auto haveCommandline = commandLine->Exists();
    const bool same = (configured.get() == commandLine.get());
    if (haveCommandline) {
        if (haveConfigured && (false == same)) {
            migrate = true;
            previous.Set(configured);
            LogOutput(OT_METHOD)(__func__)(": Migrating from ")(previous)(".")
                .Flush();
        }

        return commandLine;
    } else {
        if (haveConfigured) {
            LogDetail(OT_METHOD)(__func__)(": Using config file value.")
                .Flush();

            return configured;
        } else {
            LogDetail(OT_METHOD)(__func__)(": Using default value.").Flush();

            return hardcoded;
        }
    }
}

void StorageParent::start()
{
    OT_ASSERT(storage_);

    storage_->InitBackup();

#if OT_CRYPTO_WITH_BIP32
    if (storage_encryption_key_.get()) {
        storage_->InitEncryptedBackup(storage_encryption_key_);
    }
#endif

    storage_->start();
    storage_->UpgradeNyms();
}
}  // namespace opentxs::api::implementation
