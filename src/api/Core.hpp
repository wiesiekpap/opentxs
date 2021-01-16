// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include "api/Scheduler.hpp"
#include "api/StorageParent.hpp"
#include "api/ZMQ.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/crypto/Crypto.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
}  // namespace crypto

class Legacy;
class Settings;
}  // namespace api

namespace proto
{
class Ciphertext;
}  // namespace proto

class Flag;
class OTPassword;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Core : virtual public api::internal::Core,
             public ZMQ,
             public Scheduler,
             public api::implementation::StorageParent
{
public:
    static auto get_api(const PasswordPrompt& reason) noexcept
        -> const api::internal::Core&;

    auto Asymmetric() const -> const crypto::Asymmetric& final
    {
        return asymmetric_;
    }
    auto Config() const -> const api::Settings& final { return config_; }
    auto Crypto() const -> const api::Crypto& final { return crypto_; }
    auto DataFolder() const -> const std::string& final { return data_folder_; }
    auto DHT() const -> const api::network::Dht& final;
    auto Endpoints() const -> const api::Endpoints& final { return endpoints_; }
    auto Factory() const -> const api::Factory& final { return factory_; }
    INTERNAL_PASSWORD_CALLBACK* GetInternalPasswordCallback() const final;
    auto GetSecret(
        const opentxs::Lock& lock,
        Secret& secret,
        const PasswordPrompt& reason,
        const bool twice,
        const std::string& key) const -> bool final;
    auto Instance() const -> int final { return instance_; }
    auto Legacy() const noexcept -> const api::Legacy& final
    {
        return parent_.Legacy();
    }
    auto Lock() const -> std::mutex& final { return master_key_lock_; }
    auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::key::Symmetric& final;
    auto Seeds() const -> const api::HDSeed& final;
    void SetMasterKeyTimeout(const std::chrono::seconds& timeout) const final;
    auto Storage() const -> const api::storage::Storage& final;
    auto Symmetric() const -> const api::crypto::Symmetric& final
    {
        return symmetric_;
    }
    auto Wallet() const -> const api::Wallet& final;
    auto ZeroMQ() const -> const opentxs::network::zeromq::Context& final
    {
        return zmq_context_;
    }

    ~Core() override;

protected:
    std::unique_ptr<api::internal::Factory> factory_p_;
    const api::internal::Factory& factory_;
    const api::crypto::internal::Asymmetric& asymmetric_;
    const api::crypto::Symmetric& symmetric_;
    std::unique_ptr<api::HDSeed> seeds_;
    std::unique_ptr<api::Wallet> wallet_;
    std::unique_ptr<api::network::Dht> dht_;
    mutable std::mutex master_key_lock_;
    mutable std::optional<OTSecret> master_secret_;
    mutable OTSymmetricKey master_key_;
    mutable std::thread password_timeout_;
    mutable std::chrono::seconds password_duration_;
    mutable Time last_activity_;
    mutable std::atomic<bool> timeout_thread_running_;

    static auto make_master_key(
        const api::internal::Context& parent,
        const api::Factory& factory,
        const proto::Ciphertext& encrypted_secret_,
        std::optional<OTSecret>& master_secret_,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage) -> OTSymmetricKey;

    void cleanup();

    Core(
        const api::internal::Context& parent,
        Flag& running,
        const ArgList& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const std::string& dataFolder,
        const int instance,
        const bool dhtDefault,
        std::unique_ptr<api::internal::Factory> factory);

private:
    void bump_password_timer(const opentxs::Lock& lock) const;
    void password_timeout() const;

    void storage_gc_hook() final;

    Core() = delete;
    Core(const Core&) = delete;
    Core(Core&&) = delete;
    auto operator=(const Core&) -> Core& = delete;
    auto operator=(Core&&) -> Core& = delete;
};
}  // namespace opentxs::api::implementation
