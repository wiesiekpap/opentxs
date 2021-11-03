// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace opentxs
{
namespace api
{
namespace client
{
class Activity;
class Contacts;
}  // namespace client

namespace crypto
{
class Asymmetric;
class Blockchain;
class Config;
class Encode;
class Hash;
class Seed;
class Symmetric;
}  // namespace crypto

namespace session
{
class Factory;
class Storage;
}  // namespace session

class Crypto;
class Legacy;
class Session;
class Settings;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
class HashingProvider;
class Pbkdf2;
class Ripemd160;
class Scrypt;
}  // namespace crypto

class Options;
}  // namespace opentxs

namespace opentxs::factory
{
auto AsymmetricAPI(const api::Session& api) noexcept
    -> std::unique_ptr<api::crypto::Asymmetric>;
auto BlockchainAPI(
    const api::Session& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const Options& args) noexcept -> std::shared_ptr<api::crypto::Blockchain>;
auto CryptoAPI(const api::Settings& settings) noexcept
    -> std::unique_ptr<api::Crypto>;
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
auto SeedAPI(
    const api::Session& api,
    const api::session::Factory& factory,
    const api::crypto::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::session::Storage& storage,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39) noexcept -> std::unique_ptr<api::crypto::Seed>;
auto Symmetric(const api::Session& api) noexcept
    -> std::unique_ptr<api::crypto::Symmetric>;
}  // namespace opentxs::factory
