// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/key/asymmetric/Role.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <string_view>

#include "Proto.hpp"
#include "crypto/Seed.hpp"
#include "internal/api/crypto/Seed.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/Enums.pb.h"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Seed;
class Symmetric;
}  // namespace crypto

namespace session
{
class Factory;
class Storage;
}  // namespace session

class Session;
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
class Seed;
}  // namespace crypto

namespace proto
{
class HDPath;
class Seed;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::crypto::imp
{
class Seed final : public internal::Seed
{
public:
    auto AccountChildKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto AccountChildKey(
        const ReadView& path,
        const BIP44Chain internal,
        const Bip32Index index,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto AccountKey(
        const proto::HDPath& path,
        const BIP44Chain internal,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto AllowedSeedTypes() const noexcept -> const
        UnallocatedMap<opentxs::crypto::SeedStyle, UnallocatedCString>& final;
    auto AllowedLanguages(const opentxs::crypto::SeedStyle type) const noexcept
        -> const
        UnallocatedMap<opentxs::crypto::Language, UnallocatedCString>& final;
    auto AllowedSeedStrength(const opentxs::crypto::SeedStyle type)
        const noexcept -> const UnallocatedMap<
            opentxs::crypto::SeedStrength,
            UnallocatedCString>& final;
    auto Bip32Root(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto DefaultSeed() const -> UnallocatedCString final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetHDKey(
        const UnallocatedCString& seedID,
        const EcdsaCurve& curve,
        const UnallocatedVector<Bip32Index>& path,
        const opentxs::crypto::key::asymmetric::Role role,
        const VersionNumber version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::HD> final;
    auto GetOrCreateDefaultSeed(
        UnallocatedCString& seedID,
        opentxs::crypto::SeedStyle& type,
        opentxs::crypto::Language& lang,
        Bip32Index& index,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> OTSecret final;
    auto GetPaymentCode(
        const UnallocatedCString& seedID,
        const Bip32Index nym,
        const std::uint8_t version,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<opentxs::crypto::key::Secp256k1> final;
    auto GetSeed(
        const UnallocatedCString& seedID,
        Bip32Index& index,
        const PasswordPrompt& reason) const -> OTSecret final;
    auto GetStorageKey(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> OTSymmetricKey final;
    auto ImportRaw(const Secret& entropy, const PasswordPrompt& reason) const
        -> UnallocatedCString final;
    auto ImportSeed(
        const Secret& words,
        const Secret& passphrase,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto LongestWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> std::size_t final;
    auto NewSeed(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto Passphrase(
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const -> UnallocatedCString final;
    auto SeedDescription(UnallocatedCString seedID) const noexcept
        -> UnallocatedCString final;
    auto UpdateIndex(
        const UnallocatedCString& seedID,
        const Bip32Index index,
        const PasswordPrompt& reason) const -> bool final;
    auto ValidateWord(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const std::string_view word) const noexcept
        -> UnallocatedVector<UnallocatedCString> final;
    auto WordCount(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::SeedStrength strength) const noexcept
        -> std::size_t final;
    auto Words(const UnallocatedCString& seedID, const PasswordPrompt& reason)
        const -> UnallocatedCString final;

    Seed(
        const api::Session& api,
        const api::session::Factory& factory,
        const api::crypto::Asymmetric& asymmetric,
        const api::crypto::Symmetric& symmetric,
        const api::session::Storage& storage,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39);

    ~Seed() final = default;

private:
    using SeedMap = UnallocatedMap<UnallocatedCString, opentxs::crypto::Seed>;

    const api::Session& api_;  // WARNING do not access during construction
    const api::session::Factory& factory_;
    const api::crypto::Symmetric& symmetric_;
    const api::crypto::Asymmetric& asymmetric_;
    const api::session::Storage& storage_;
    const opentxs::crypto::Bip32& bip32_;
    const opentxs::crypto::Bip39& bip39_;
    mutable std::mutex seed_lock_;
    mutable SeedMap seeds_;

    auto get_seed(
        const Lock& lock,
        const UnallocatedCString& seedID,
        const PasswordPrompt& reason) const noexcept(false)
        -> opentxs::crypto::Seed&;
    auto new_seed(
        const Lock& lock,
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang,
        const opentxs::crypto::SeedStrength strength,
        const PasswordPrompt& reason) const noexcept -> UnallocatedCString;

    Seed() = delete;
    Seed(const Seed&) = delete;
    Seed(Seed&&) = delete;
    auto operator=(const Seed&) -> Seed& = delete;
    auto operator=(Seed&&) -> Seed& = delete;
};
}  // namespace opentxs::api::crypto::imp
