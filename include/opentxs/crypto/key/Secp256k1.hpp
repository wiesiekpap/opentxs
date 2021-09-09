// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_SECP256K1_HPP
#define OPENTXS_CRYPTO_KEY_SECP256K1_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/crypto/key/HD.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class Secp256k1 : virtual public HD
{
public:
    ~Secp256k1() override = default;

protected:
    Secp256k1() = default;

private:
    Secp256k1(const Secp256k1&) = delete;
    Secp256k1(Secp256k1&&) = delete;
    auto operator=(const Secp256k1&) -> Secp256k1& = delete;
    auto operator=(Secp256k1&&) -> Secp256k1& = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
