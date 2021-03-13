// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_CRYPTO_HPP
#define OPENTXS_API_CRYPTO_CRYPTO_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace api
{
class Crypto
{
public:
    OPENTXS_EXPORT virtual const crypto::Config& Config() const = 0;

    // Encoding function interface
    OPENTXS_EXPORT virtual const crypto::Encode& Encode() const = 0;

    // Hash function interface
    OPENTXS_EXPORT virtual const crypto::Hash& Hash() const = 0;

    // Utility class for misc OpenSSL-provided functions
    OPENTXS_EXPORT virtual const crypto::Util& Util() const = 0;

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    OPENTXS_EXPORT virtual const opentxs::crypto::EcdsaProvider& ED25519()
        const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    OPENTXS_EXPORT virtual const opentxs::crypto::AsymmetricProvider& RSA()
        const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual const opentxs::crypto::EcdsaProvider& SECP256K1()
        const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

    OPENTXS_EXPORT virtual const opentxs::crypto::SymmetricProvider& Sodium()
        const = 0;
    OPENTXS_EXPORT virtual const opentxs::crypto::Bip32& BIP32() const = 0;
    OPENTXS_EXPORT virtual const opentxs::crypto::Bip39& BIP39() const = 0;

    OPENTXS_EXPORT virtual ~Crypto() = default;

protected:
    Crypto() = default;

private:
    Crypto(const Crypto&) = delete;
    Crypto(Crypto&&) = delete;
    Crypto& operator=(const Crypto&) = delete;
    Crypto& operator=(Crypto&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
