// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/HDSeed.cpp"

#pragma once

#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/protobuf/Enums.pb.h"

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

class Symmetric;
}  // namespace crypto

namespace storage
{
class Storage;
}  // namespace storage

class Factory;
}  // namespace api

namespace crypto
{
namespace key
{
class HD;
class Secp256k1;
}  // namespace key

class Bip32;
class Bip39;
}  // namespace crypto

namespace proto
{
class HDPath;
class Seed;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::implementation
{
class HDSeed final : public api::HDSeed
{
public:
#if OT_CRYPTO_WITH_BIP32
    auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto AllowedSeedTypes() const noexcept -> const SupportedSeeds& final;
    auto AllowedLanguages(const Style type) const noexcept
        -> const SupportedLanguages& final;
    auto AllowedSeedStrength(const Style type) const noexcept
        -> const SupportedStrengths& final;
    auto Bip32Root(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const -> std::string final;
    auto DefaultSeed() const -> std::string final;
#if OT_CRYPTO_WITH_BIP32
    auto GetHDKey(
        std::string& fingerprint,
        const EcdsaCurve& curve,
        const Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto GetOrCreateDefaultSeed(
        std::string& fingerprint,
        Style& type,
        Language& lang,
        Bip32Index& index,
        const Strength strength,
        const PasswordPrompt& reason) const -> OTSecret final;
#if OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    auto GetStorageKey(std::string& seed, const PasswordPrompt& reason) const
        -> OTSymmetricKey final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason) const
        -> std::string final;
    auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const Style type,
        const Language lang,
        const PasswordPrompt& reason) const -> std::string final;
    auto LongestWord(const Style type, const Language lang) const noexcept
        -> std::size_t final;
    auto NewSeed(
        const Style type,
        const Language lang,
        const Strength strength,
        const PasswordPrompt& reason) const -> std::string final;
    auto Passphrase(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const -> std::string final;
    auto Seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> OTSecret final;
    auto UpdateIndex(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool final;
    auto ValidateWord(
        const Style type,
        const Language lang,
        const std::string_view word) const noexcept -> Matches final;
    auto WordCount(const Style type, const Strength strength) const noexcept
        -> std::size_t final;
    auto Words(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const -> std::string final;

    HDSeed(
        const api::Factory& factory,
        const api::crypto::internal::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::storage::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39);

    ~HDSeed() final = default;

private:
    using SeedMap = std::map<Style, proto::SeedType>;
    using SeedReverseMap = std::map<proto::SeedType, Style>;
    using LangMap = std::map<Language, proto::SeedLang>;
    using LangReverseMap = std::map<proto::SeedLang, Language>;

    static const std::string DEFAULT_PASSPHRASE;
    static const proto::SymmetricMode DEFAULT_ENCRYPTION_MODE;
    static const VersionNumber DefaultVersion{4};
    static const SeedMap seed_map_;
    static const SeedReverseMap seed_reverse_map_;
    static const LangMap lang_map_;
    static const LangReverseMap lang_reverse_map_;

    const api::crypto::Symmetric& symmetric_;
#if OT_CRYPTO_WITH_BIP32
    const api::crypto::internal::Asymmetric& asymmetric_;
#endif  // OT_CRYPTO_WITH_BIP32
    const api::storage::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;
    const OTSecret binary_secret_;
    const OTSecret text_secret_;
    mutable std::mutex lock_;

    static auto translate(const Style in) noexcept -> proto::SeedType
    {
        return seed_map_.at(in);
    }
    static auto translate(const proto::SeedType in) noexcept -> Style
    {
        return seed_reverse_map_.at(in);
    }
    static auto translate(const Language in) noexcept -> proto::SeedLang
    {
        return lang_map_.at(in);
    }
    static auto translate(const proto::SeedLang in) noexcept -> Language
    {
        return lang_reverse_map_.at(in);
    }

    auto decrypt_seed(
        const proto::Seed& seed,
        Secret& words,
        Secret& phrase,
        Secret& raw,
        const PasswordPrompt& reason) const -> bool;
    auto load_seed(
        std::string& fingerprint,
        Bip32Index& index,
        Style& type,
        Language& lang,
        const PasswordPrompt& reason) const -> OTSecret;
    auto save_seed(
        const Secret& words,
        const Secret& passphrase,
        const Secret& raw,
        const Style type,
        const Language lang,
        const PasswordPrompt& reason) const -> std::string;
    auto seed_to_data(
        const Secret& words,
        const Secret& passphrase,
        const Secret& raw,
        Secret& output) const -> bool;
    auto serialized_seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> std::shared_ptr<proto::Seed>;

    HDSeed() = delete;
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    auto operator=(const HDSeed&) -> HDSeed& = delete;
    auto operator=(HDSeed&&) -> HDSeed& = delete;
};
}  // namespace opentxs::api::implementation
