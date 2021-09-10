// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_CRYPTO_HPP
#define OPENTXS_API_CRYPTO_CRYPTO_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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
}  // namespace api

namespace crypto
{
class AsymmetricProvider;
class Bip32;
class Bip39;
class EcdsaProvider;
class SymmetricProvider;
}  // namespace crypto
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Crypto
{
public:
    virtual auto Config() const -> const crypto::Config& = 0;

    // Encoding function interface
    virtual auto Encode() const -> const crypto::Encode& = 0;

    // Hash function interface
    virtual auto Hash() const -> const crypto::Hash& = 0;

    // Utility class for misc OpenSSL-provided functions
    virtual auto Util() const -> const crypto::Util& = 0;

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    virtual auto ED25519() const -> const opentxs::crypto::EcdsaProvider& = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    virtual auto RSA() const -> const opentxs::crypto::AsymmetricProvider& = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    virtual auto SECP256K1() const -> const opentxs::crypto::EcdsaProvider& = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    virtual auto Sodium() const
        -> const opentxs::crypto::SymmetricProvider& = 0;
    virtual auto BIP32() const -> const opentxs::crypto::Bip32& = 0;
    virtual auto BIP39() const -> const opentxs::crypto::Bip39& = 0;

    virtual ~Crypto() = default;

protected:
    Crypto() = default;

private:
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    auto operator=(const Crypto&) -> Crypto& = delete;
    auto operator=(Crypto&&) -> Crypto& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
