// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
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
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Dht.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"

class QObject;

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
}  // namespace crypto

class Core;
class Legacy;
class Settings;
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Ciphertext;
}  // namespace proto

class Flag;
class OTPassword;
class Options;
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
        -> const api::Core&;

    auto Asymmetric() const noexcept -> const crypto::Asymmetric& final
    {
        return asymmetric_;
    }
    auto Config() const noexcept -> const api::Settings& final
    {
        return config_;
    }
    auto Crypto() const noexcept -> const api::Crypto& final { return crypto_; }
    auto DataFolder() const noexcept -> const std::string& final
    {
        return data_folder_;
    }
    auto Endpoints() const noexcept -> const api::Endpoints& final
    {
        return endpoints_;
    }
    auto Factory() const noexcept -> const api::Factory& final
    {
        return factory_;
    }
    auto GetInternalPasswordCallback() const
        -> INTERNAL_PASSWORD_CALLBACK* final;
    auto GetOptions() const noexcept -> const Options& final { return args_; }
    auto GetSecret(
        const opentxs::Lock& lock,
        Secret& secret,
        const PasswordPrompt& reason,
        const bool twice,
        const std::string& key) const -> bool final;
    auto Instance() const noexcept -> int final { return instance_; }
    auto Internal() const noexcept -> internal::Core& final
    {
        return const_cast<Core&>(*this);
    }
    auto Legacy() const noexcept -> const api::Legacy& final
    {
        return parent_.Legacy();
    }
    auto Lock() const -> std::mutex& final { return master_key_lock_; }
    auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::key::Symmetric& final;
    auto NewNym(const identifier::Nym& id) const noexcept -> void override {}
    auto Network() const noexcept -> const network::Network& final
    {
        return network_;
    }
    auto QtRootObject() const noexcept -> QObject* final
    {
        return parent_.QtRootObject();
    }
    auto Seeds() const noexcept -> const api::HDSeed& final;
    auto SetMasterKeyTimeout(const std::chrono::seconds& timeout) const noexcept
        -> void final;
    auto Storage() const noexcept -> const api::storage::Storage& final;
    auto Symmetric() const noexcept -> const api::crypto::Symmetric& final
    {
        return symmetric_;
    }
    auto Wallet() const noexcept -> const api::Wallet& final;

    ~Core() override;

protected:
    network::Network network_;
    std::unique_ptr<api::internal::Factory> factory_p_;
    const api::internal::Factory& factory_;
    const api::crypto::internal::Asymmetric& asymmetric_;
    const api::crypto::Symmetric& symmetric_;
    std::unique_ptr<api::HDSeed> seeds_;
    std::unique_ptr<api::Wallet> wallet_;

private:
    proto::Ciphertext encrypted_secret_;

protected:
    using NetworkMaker = std::function<api::network::Network::Imp*(
        const opentxs::network::zeromq::Context& zmq,
        const api::Endpoints& endpoints,
        api::implementation::Scheduler& config)>;

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
        proto::Ciphertext& encrypted_secret_,
        std::optional<OTSecret>& master_secret_,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage) -> OTSymmetricKey;

    void cleanup();

    Core(
        const api::internal::Context& parent,
        Flag& running,
        Options&& args,
        const api::Crypto& crypto,
        const api::Settings& config,
        const opentxs::network::zeromq::Context& zmq,
        const std::string& dataFolder,
        const int instance,
        NetworkMaker network,
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
