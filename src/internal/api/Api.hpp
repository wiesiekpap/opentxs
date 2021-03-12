// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/ThreadPool.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
namespace internal
{
struct Asymmetric;
}  // namespace internal
}  // namespace crypto

class Crypto;
class Endpoints;
class HDSeed;
class Legacy;
class Primitives;
class Settings;
}  // namespace api
}  // namespace opentxs

namespace
{
/** Callbacks in this form allow OpenSSL to query opentxs to get key encryption
 *  and decryption passwords*/
extern "C" {
using INTERNAL_PASSWORD_CALLBACK =
    std::int32_t(char*, std::int32_t, std::int32_t, void*);
}
}  // namespace

namespace opentxs::api::internal
{
struct Context : virtual public api::Context {
    virtual auto GetPasswordCaller() const -> OTCaller& = 0;
    virtual void Init() = 0;
    virtual auto Legacy() const noexcept -> const api::Legacy& = 0;
    virtual void shutdown() = 0;

    ~Context() override = default;
};

struct Core : virtual public api::Core {
    const proto::Ciphertext encrypted_secret_{};

    virtual auto GetInternalPasswordCallback() const
        -> INTERNAL_PASSWORD_CALLBACK* = 0;
    virtual auto GetSecret(
        const opentxs::Lock& lock,
        Secret& secret,
        const PasswordPrompt& reason,
        const bool twice,
        const std::string& key = "") const -> bool = 0;
    virtual auto Legacy() const noexcept -> const api::Legacy& = 0;
    virtual auto Lock() const -> std::mutex& = 0;
    virtual auto MasterKey(const opentxs::Lock& lock) const
        -> const opentxs::crypto::key::Symmetric& = 0;

    ~Core() override = default;
};

struct Factory : virtual public api::Factory {
    virtual auto Asymmetric() const
        -> const api::crypto::internal::Asymmetric& = 0;
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;

    ~Factory() override = default;
};

struct Log {
    virtual ~Log() = default;
};

struct ThreadPool : virtual public api::ThreadPool {
    enum class Work : OTZMQWorkType {
        BlockchainWallet = OT_ZMQ_INTERNAL_SIGNAL + 0,
        SyncDataFiltersIncoming = OT_ZMQ_INTERNAL_SIGNAL + 1,
        CalculateBlockFilters = OT_ZMQ_INTERNAL_SIGNAL + 2,
    };

    virtual auto Shutdown() noexcept -> void = 0;

    ~ThreadPool() override = default;
};

constexpr auto value(ThreadPool::Work in) noexcept
{
    return static_cast<OTZMQWorkType>(in);
}
}  // namespace opentxs::api::internal
