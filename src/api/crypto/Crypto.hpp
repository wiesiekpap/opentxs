// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>

#include "Proto.hpp"
#include "internal/api/Api.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Config;
class Encode;
class Hash;
class Util;
}  // namespace crypto

class Primitives;
class Settings;
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key

class OpenSSL;
class Ripemd160;
class Secp256k1;
class Sodium;
}  // namespace crypto
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Crypto final : public internal::Crypto
{
public:
    auto Config() const -> const crypto::Config& final;
    auto Encode() const -> const crypto::Encode& final;
    auto Hash() const -> const crypto::Hash& final;
    auto Init(const api::Primitives& factory) noexcept -> void final;
    auto Util() const -> const crypto::Util& final;
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    auto ED25519() const -> const opentxs::crypto::EcdsaProvider& final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    auto RSA() const -> const opentxs::crypto::AsymmetricProvider& final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto SECP256K1() const -> const opentxs::crypto::EcdsaProvider& final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto Sodium() const -> const opentxs::crypto::SymmetricProvider& final;
    auto BIP32() const -> const opentxs::crypto::Bip32& final;
    auto BIP39() const -> const opentxs::crypto::Bip39& final;

    Crypto(const api::Settings& settings) noexcept;

    ~Crypto() final;

private:
    std::unique_ptr<api::crypto::Config> config_;
    std::unique_ptr<opentxs::crypto::Sodium> sodium_;
#if OT_CRYPTO_USING_OPENSSL
    std::unique_ptr<opentxs::crypto::OpenSSL> ssl_;
#endif  // OT_CRYPTO_USING_OPENSSL
    const api::crypto::Util& util_;
#if OT_CRYPTO_USING_LIBSECP256K1
    std::unique_ptr<opentxs::crypto::Secp256k1> secp256k1_p_;
#endif  // OT_CRYPTO_USING_LIBSECP256K1
    std::unique_ptr<opentxs::crypto::Bip39> bip39_p_;
    const opentxs::crypto::Ripemd160& ripemd160_;
    opentxs::crypto::Bip32 bip32_;
    const opentxs::crypto::Bip39& bip39_;
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    const opentxs::crypto::EcdsaProvider& secp256k1_;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    std::unique_ptr<crypto::Encode> encode_;
    std::unique_ptr<crypto::Hash> hash_;

    void Init();
    void Cleanup();

    Crypto() = delete;
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    auto operator=(const Crypto&) -> Crypto& = delete;
    auto operator=(Crypto&&) -> Crypto& = delete;
};
}  // namespace opentxs::api::implementation
