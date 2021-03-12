// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

namespace opentxs
{
namespace api
{
namespace crypto
{
class Util;
}  // namespace crypto

class Crypto;
}  // namespace api

namespace crypto
{
class OpenSSL;
class Ripemd160;
class Secp256k1;
class Sodium;
}  // namespace crypto
}  // namespace opentxs

namespace opentxs::factory
{
#if OT_CRYPTO_USING_OPENSSL
auto OpenSSL() noexcept -> std::unique_ptr<crypto::OpenSSL>;
#endif  // OT_CRYPTO_USING_OPENSSL
#if OT_CRYPTO_USING_LIBSECP256K1
auto Secp256k1(
    const api::Crypto& crypto,
    const api::crypto::Util& util) noexcept
    -> std::unique_ptr<crypto::Secp256k1>;
#endif  // OT_CRYPTO_USING_LIBSECP256K1
auto Sodium(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<crypto::Sodium>;
}  // namespace opentxs::factory
