// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_HDSEED_HPP
#define OPENTXS_API_HDSEED_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"

namespace opentxs
{
class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT HDSeed
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

    OPENTXS_NO_EXPORT virtual auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto AccountChildKey(
        const ReadView& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    OPENTXS_NO_EXPORT virtual auto AccountKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto AllowedSeedTypes() const noexcept -> const SupportedSeeds& = 0;
    virtual auto AllowedLanguages(const Style type) const noexcept
        -> const SupportedLanguages& = 0;
    virtual auto AllowedSeedStrength(const Style type) const noexcept
        -> const SupportedStrengths& = 0;
    virtual auto Bip32Root(
        const std::string& seedID,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto DefaultSeed() const -> std::string = 0;
    virtual auto GetHDKey(
        const std::string& seedID,
        const EcdsaCurve& curve,
        const Path& path,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const
        -> std::unique_ptr<opentxs::crypto::key::HD> = 0;
    virtual auto GetOrCreateDefaultSeed(
        std::string& seedID,
        Style& type,
        Language& lang,
        Bip32Index& index,
        const Strength strength,
        const PasswordPrompt& reason) const -> OTSecret = 0;
    virtual auto GetPaymentCode(
        const std::string& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> = 0;
    virtual auto GetStorageKey(
        const std::string& seedID,
        const PasswordPrompt& reason) const -> OTSymmetricKey = 0;
    virtual auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason)
        const -> std::string = 0;
    virtual auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const Style type,
        const Language lang,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto LongestWord(const Style type, const Language lang)
        const noexcept -> std::size_t = 0;
    virtual auto NewSeed(
        const Style type,
        const Language lang,
        const Strength strength,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto Passphrase(
        const std::string& seedID,
        const PasswordPrompt& reason) const -> std::string = 0;
    virtual auto Seed(
        const std::string& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> OTSecret = 0;
    virtual auto SeedDescription(std::string seedID) const noexcept
        -> std::string = 0;
    virtual auto UpdateIndex(
        const std::string& seedID,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto ValidateWord(
        const Style type,
        const Language lang,
        const std::string_view word) const noexcept -> Matches = 0;
    virtual auto WordCount(const Style type, const Strength strength)
        const noexcept -> std::size_t = 0;
    virtual auto Words(const std::string& seedID, const PasswordPrompt& reason)
        const -> std::string = 0;

    OPENTXS_NO_EXPORT virtual ~HDSeed() = default;

protected:
    HDSeed() = default;

private:
    HDSeed(const HDSeed&) = delete;
    HDSeed(HDSeed&&) = delete;
    auto operator=(const HDSeed&) -> HDSeed& = delete;
    auto operator=(HDSeed&&) -> HDSeed& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
