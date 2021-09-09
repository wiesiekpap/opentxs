// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "api/crypto/Crypto.hpp"  // IWYU pragma: associated

#ifndef _WIN32
extern "C" {
#include <sys/resource.h>
}
#endif

#include "2_Factory.hpp"
#include "internal/api/crypto/Factory.hpp"
#include "internal/crypto/Crypto.hpp"
#include "internal/crypto/Factory.hpp"
#include "internal/crypto/library/Factory.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "internal/crypto/library/OpenSSL.hpp"
#endif  // OT_CRYPTO_USING_OPENSSL
#if OT_CRYPTO_USING_LIBSECP256K1
#include "internal/crypto/library/Secp256k1.hpp"
#endif  // OT_CRYPTO_USING_LIBSECP256K1
#include "internal/crypto/library/Sodium.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

#define OT_METHOD "opentxs::Crypto::"

template class opentxs::Pimpl<opentxs::crypto::key::Symmetric>;

namespace opentxs::factory
{
auto CryptoAPI(const api::Settings& settings) noexcept
    -> std::unique_ptr<api::internal::Crypto>
{
    using ReturnType = api::implementation::Crypto;

    return std::make_unique<ReturnType>(settings);
}
}  // namespace opentxs::factory

namespace opentxs::api::implementation
{
Crypto::Crypto(const api::Settings& settings) noexcept
    : config_(factory::CryptoConfig(settings))
    , sodium_(factory::Sodium(*this))
#if OT_CRYPTO_USING_OPENSSL
    , ssl_(factory::OpenSSL())
#endif  // OT_CRYPTO_USING_OPENSSL
    , util_(*sodium_)
#if OT_CRYPTO_USING_LIBSECP256K1
    , secp256k1_p_(factory::Secp256k1(*this, util_))
#endif  // OT_CRYPTO_USING_LIBSECP256K1
    , bip39_p_(opentxs::Factory::Bip39(*this))
#if OT_CRYPTO_USING_OPENSSL
    , ripemd160_(*ssl_)
#else   // OT_CRYPTO_USING_OPENSSL
    , ripemd160_(*sodium_)
#endif  // OT_CRYPTO_USING_OPENSSL
    , bip32_(factory::Bip32(*this))
    , bip39_(*bip39_p_)
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_USING_LIBSECP256K1
    , secp256k1_(*secp256k1_p_)
#endif  // OT_CRYPTO_USING_LIBSECP256K1
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    , encode_(factory::Encode(*this))
    , hash_(factory::Hash(
          *encode_,
#if OT_CRYPTO_USING_OPENSSL
          *ssl_,
#else   // OT_CRYPTO_USING_OPENSSL
          *sodium_,
#endif  // OT_CRYPTO_USING_OPENSSL
          *sodium_,
#if OT_CRYPTO_USING_OPENSSL
          *ssl_,
#else   // OT_CRYPTO_USING_OPENSSL
          *sodium_,
#endif  // OT_CRYPTO_USING_OPENSSL
          ripemd160_,
          *sodium_))
{
    OT_ASSERT(sodium_)
#if OT_CRYPTO_USING_OPENSSL
    OT_ASSERT(ssl_)
#endif  // OT_CRYPTO_USING_OPENSSL
#if OT_CRYPTO_USING_LIBSECP256K1
    OT_ASSERT(secp256k1_p_)
#endif  // OT_CRYPTO_USING_LIBSECP256K1

    Init();
}

auto Crypto::BIP32() const -> const opentxs::crypto::Bip32& { return bip32_; }

auto Crypto::BIP39() const -> const opentxs::crypto::Bip39& { return bip39_; }

void Crypto::Cleanup()
{
    hash_.reset();
    encode_.reset();
#if OT_CRYPTO_USING_LIBSECP256K1
    secp256k1_p_.reset();
#endif  // OT_CRYPTO_USING_LIBSECP256K1
    sodium_.reset();
    config_.reset();
}

auto Crypto::Config() const -> const crypto::Config&
{
    OT_ASSERT(config_);

    return *config_;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
auto Crypto::ED25519() const -> const opentxs::crypto::EcdsaProvider&
{
    OT_ASSERT(sodium_);

    return *sodium_;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

auto Crypto::Encode() const -> const crypto::Encode&
{
    OT_ASSERT(encode_);

    return *encode_;
}

auto Crypto::Hash() const -> const crypto::Hash&
{
    OT_ASSERT(hash_);

    return *hash_;
}

void Crypto::Init()
{
    LogDetail(OT_METHOD)(__func__)(
        ": Setting up rlimits, and crypto libraries...")
        .Flush();

// Here is a security measure intended to make it more difficult to
// capture a core dump. (Not used in debug mode, obviously.)
//
#if !defined(PREDEF_MODE_DEBUG) && defined(PREDEF_PLATFORM_UNIX)
    struct rlimit rlim;
    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = rlim.rlim_cur = 0;
    if (setrlimit(RLIMIT_CORE, &rlim)) {
        OT_FAIL_MSG("Crypto::Init: ASSERT: setrlimit failed. (Used for "
                    "preventing core dumps.)\n");
    }
#endif  // !defined(PREDEF_MODE_DEBUG) && defined(PREDEF_PLATFORM_UNIX)

#if OT_CRYPTO_USING_LIBSECP256K1
    secp256k1_p_->Init();
#endif  // OT_CRYPTO_USING_LIBSECP256K1
}

auto Crypto::Init(const api::Primitives& factory) noexcept -> void
{
    bip32_.Internal().Init(factory);
}

#if OT_CRYPTO_SUPPORTED_KEY_RSA
auto Crypto::RSA() const -> const opentxs::crypto::AsymmetricProvider&
{
    OT_ASSERT(nullptr != ssl_);

    return *ssl_;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto Crypto::SECP256K1() const -> const opentxs::crypto::EcdsaProvider&
{
    return secp256k1_;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

auto Crypto::Sodium() const -> const opentxs::crypto::SymmetricProvider&
{
    OT_ASSERT(sodium_);

    return *sodium_;
}

auto Crypto::Util() const -> const crypto::Util& { return util_; }

Crypto::~Crypto() { Cleanup(); }
}  // namespace opentxs::api::implementation
