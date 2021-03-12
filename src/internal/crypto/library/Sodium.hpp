// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/crypto/library/Pbkdf2.hpp"
#include "internal/crypto/library/Ripemd160.hpp"
#include "opentxs/api/crypto/Util.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "internal/crypto/library/Scrypt.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"

namespace opentxs::crypto
{
class Sodium : virtual public api::crypto::Util
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
               virtual public EcdsaProvider
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
    ,
               virtual public HashingProvider,
               virtual public SymmetricProvider,
               virtual public Scrypt,
               virtual public Ripemd160,
               virtual public Pbkdf2
{
public:
    ~Sodium() override = default;

protected:
    Sodium() = default;

private:
    Sodium(const Sodium&) = delete;
    Sodium(Sodium&&) = delete;
    auto operator=(const Sodium&) -> Sodium& = delete;
    auto operator=(Sodium&&) -> Sodium& = delete;
};
}  // namespace opentxs::crypto
