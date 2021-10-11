// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "api/StorageParent.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/api/storage/Factory.hpp"
#include "internal/api/storage/Storage.hpp"
#include "opentxs/Pimpl.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#endif  // OT_CRYPTO_WITH_BIP32
#include "opentxs/api/Options.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"

#if OT_CRYPTO_WITH_BIP32
#define OT_METHOD "opentxs::api::implementation::StorageParent::"
#endif

namespace opentxs::api::implementation
{
StorageParent::StorageParent(
    const Flag& running,
    Options&& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const api::Legacy& legacy,
    const api::network::Asio& asio,
    const std::string& dataFolder)
    : crypto_(crypto)
    , config_(config)
    , args_(std::move(args))
    , data_folder_(dataFolder)
    , storage_config_(legacy, config_, args_, String::Factory(dataFolder))
    , storage_(opentxs::factory::StorageInterface(
          crypto_,
          asio,
          running,
          storage_config_))
#if OT_CRYPTO_WITH_BIP32
    , storage_encryption_key_(opentxs::crypto::key::Symmetric::Factory())
#endif
{
    OT_ASSERT(storage_);
}

void StorageParent::init(
    [[maybe_unused]] const api::Factory& factory
#if OT_CRYPTO_WITH_BIP32
    ,
    const api::HDSeed& seeds
#endif  // OT_CRYPTO_WITH_BIP32
)
{
    if (storage_config_.fs_encrypted_backup_directory_.empty()) { return; }

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

StorageParent::~StorageParent() = default;
}  // namespace opentxs::api::implementation
