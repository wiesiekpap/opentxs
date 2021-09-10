// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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

class Config;
class Encode;
class Hash;
class Symmetric;
}  // namespace crypto

namespace internal
{
struct Crypto;
}  // namespace internal

class Core;
class Crypto;
class Settings;
}  // namespace api

namespace crypto
{
class HashingProvider;
class Pbkdf2;
class Ripemd160;
class Scrypt;
}  // namespace crypto
}  // namespace opentxs

namespace opentxs::factory
{
auto AsymmetricAPI(const api::Core& api) noexcept
    -> std::unique_ptr<api::crypto::internal::Asymmetric>;
auto CryptoAPI(const api::Settings& settings) noexcept
    -> std::unique_ptr<api::internal::Crypto>;
auto CryptoConfig(const api::Settings& settings) noexcept
    -> std::unique_ptr<api::crypto::Config>;
auto Encode(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<api::crypto::Encode>;
auto Hash(
    const api::crypto::Encode& encode,
    const crypto::HashingProvider& sha,
    const crypto::HashingProvider& blake,
    const crypto::Pbkdf2& pbkdf2,
    const crypto::Ripemd160& ripe,
    const crypto::Scrypt& scrypt) noexcept
    -> std::unique_ptr<api::crypto::Hash>;
auto Symmetric(const api::Core& api) noexcept
    -> std::unique_ptr<api::crypto::Symmetric>;
}  // namespace opentxs::factory
