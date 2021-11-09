// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "api/session/base/Storage.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/api/session/Crypto.hpp"
#include "internal/api/session/Factory.hpp"
#include "internal/api/session/Storage.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Pimpl.hpp"
#if OT_CRYPTO_WITH_BIP32
#include "opentxs/api/crypto/Seed.hpp"
#endif  // OT_CRYPTO_WITH_BIP32
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::api::session::base
{
Storage::Storage(
    const Flag& running,
    Options&& args,
    const api::Session& session,
    const api::Crypto& crypto,
    const api::Settings& config,
    const api::Legacy& legacy,
    const api::network::Asio& asio,
    const std::string& dataFolder,
    std::unique_ptr<api::session::Factory> factory)
    : config_(config)
    , args_(std::move(args))
    , data_folder_(dataFolder)
    , storage_config_(legacy, config_, args_, String::Factory(dataFolder))
    , factory_p_(std::move(factory))
    , storage_(
          opentxs::factory::StorageAPI(crypto, asio, running, storage_config_))
    , crypto_p_(factory::SessionCryptoAPI(
          const_cast<api::Crypto&>(crypto),
          session,
          *factory_p_,
          *storage_))
    , factory_(*factory_p_)
    , crypto_(*crypto_p_)
#if OT_CRYPTO_WITH_BIP32
    , storage_encryption_key_(opentxs::crypto::key::Symmetric::Factory())
#endif
{
    OT_ASSERT(storage_);
}

auto Storage::cleanup() noexcept -> void
{
    crypto_p_->InternalSession().Cleanup();
    factory_p_.reset();
}

auto Storage::init(
    [[maybe_unused]] const api::session::Factory& factory
#if OT_CRYPTO_WITH_BIP32
    ,
    const api::crypto::Seed& seeds
#endif  // OT_CRYPTO_WITH_BIP32
    ) noexcept -> void
{
    if (storage_config_.fs_encrypted_backup_directory_.empty()) { return; }

#if OT_CRYPTO_WITH_BIP32
    auto seed = seeds.DefaultSeed();

    if (seed.empty()) {
        LogError()(OT_PRETTY_CLASS())("No default seed.").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Default seed is: ")(seed)(".").Flush();
    }

    auto reason = factory.PasswordPrompt("Initializaing storage encryption");
    storage_encryption_key_ = seeds.GetStorageKey(seed, reason);

    if (storage_encryption_key_.get()) {
        LogDetail()(OT_PRETTY_CLASS())("Obtained storage key ")(
            storage_encryption_key_->ID(reason))
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to load storage key ")(seed)(".")
            .Flush();
    }
#endif

    start();
}

auto Storage::start() noexcept -> void
{
    OT_ASSERT(storage_);

    auto& storage = storage_->Internal();

    storage.InitBackup();

#if OT_CRYPTO_WITH_BIP32
    if (storage_encryption_key_.get()) {
        storage.InitEncryptedBackup(storage_encryption_key_);
    }
#endif

    storage.start();
    storage.UpgradeNyms();
}

Storage::~Storage() = default;
}  // namespace opentxs::api::session::base
