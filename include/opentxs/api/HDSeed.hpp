// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_HDSEED_HPP
#define OPENTXS_API_HDSEED_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Symmetric.hpp"

namespace opentxs
{
class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class HDSeed
{
public:
    using Path = std::vector<Bip32Index>;
    using Style = opentxs::crypto::SeedStyle;
    using Language = opentxs::crypto::Language;
    using Strength = opentxs::crypto::SeedStrength;
    using SupportedSeeds = std::map<Style, std::string>;
    using SupportedLanguages = std::map<Language, std::string>;
    using SupportedStrengths = std::map<Strength, std::string>;
    using Matches = std::vector<std::string>;

#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::crypto::key::HD>
    AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual const SupportedSeeds& AllowedSeedTypes()
        const noexcept = 0;
    OPENTXS_EXPORT virtual const SupportedLanguages& AllowedLanguages(
        const Style type) const noexcept = 0;
    OPENTXS_EXPORT virtual const SupportedStrengths& AllowedSeedStrength(
        const Style type) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Bip32Root(
        const PasswordPrompt& reason,
        const std::string& fingerprint = "") const = 0;
    OPENTXS_EXPORT virtual std::string DefaultSeed() const = 0;
#if OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::crypto::key::HD> GetHDKey(
        std::string& fingerprint,
        const EcdsaCurve& curve,
        const Path& path,
        const PasswordPrompt& reason,
        const proto::KeyRole role = proto::KEYROLE_SIGN,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual OTSecret GetOrCreateDefaultSeed(
        std::string& fingerprint,
        Style& type,
        Language& lang,
        Bip32Index& index,
        const Strength strength,
        const PasswordPrompt& reason) const = 0;
#if OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual std::unique_ptr<opentxs::crypto::key::Secp256k1>
    GetPaymentCode(
        std::string& fingerprint,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const = 0;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
    OPENTXS_EXPORT virtual OTSymmetricKey GetStorageKey(
        std::string& seed,
        const PasswordPrompt& reason) const = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    OPENTXS_EXPORT virtual std::string ImportRaw(
        const Secret& entropy,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual std::string ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const Style type,
        const Language lang,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual std::size_t LongestWord(
        const Style type,
        const Language lang) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string NewSeed(
        const Style type,
        const Language lang,
        const Strength strength,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual std::string Passphrase(
        const PasswordPrompt& reason,
        const std::string& fingerprint) const = 0;
    OPENTXS_EXPORT virtual OTSecret Seed(
        std::string& fingerprint,
        Bip32Index& index,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual bool UpdateIndex(
        std::string& seed,
        const Bip32Index index,
        const PasswordPrompt& reason) const = 0;
    OPENTXS_EXPORT virtual Matches ValidateWord(
        const Style type,
        const Language lang,
        const std::string_view word) const noexcept = 0;
    OPENTXS_EXPORT virtual std::size_t WordCount(
        const Style type,
        const Strength strength) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Words(
        const PasswordPrompt& reason,
        const std::string& fingerprint) const = 0;

    OPENTXS_EXPORT virtual ~HDSeed() = default;

protected:
    HDSeed() = default;

private:
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    HDSeed& operator=(const HDSeed&) = delete;
    HDSeed& operator=(HDSeed&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
