// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_ASYMMETRIC_HPP
#define OPENTXS_API_CRYPTO_ASYMMETRIC_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"

namespace opentxs
{
namespace proto
{
class AsymmetricKey;
}  // namespace proto

class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace crypto
{
class OPENTXS_EXPORT Asymmetric
{
public:
    using ECKey = std::unique_ptr<opentxs::crypto::key::EllipticCurve>;
    using Key = std::unique_ptr<opentxs::crypto::key::Asymmetric>;
    using HDKey = std::unique_ptr<opentxs::crypto::key::HD>;
    using Secp256k1Key = std::unique_ptr<opentxs::crypto::key::Secp256k1>;

    OPENTXS_NO_EXPORT virtual auto InstantiateECKey(
        const proto::AsymmetricKey& serialized) const -> ECKey = 0;
    OPENTXS_NO_EXPORT virtual auto InstantiateHDKey(
        const proto::AsymmetricKey& serialized) const -> HDKey = 0;
    virtual auto InstantiateKey(
        const opentxs::crypto::key::asymmetric::Algorithm type,
        const std::string& seedID,
        const opentxs::crypto::Bip32::Key& serialized,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const
        -> HDKey = 0;
    OPENTXS_NO_EXPORT virtual auto InstantiateKey(
        const proto::AsymmetricKey& serialized) const -> Key = 0;
    virtual auto NewHDKey(
        const std::string& seedID,
        const Secret& seed,
        const EcdsaCurve& curve,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::EllipticCurve::DefaultVersion) const
        -> HDKey = 0;
    virtual auto InstantiateSecp256k1Key(
        const ReadView publicKey,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Secp256k1::DefaultVersion) const noexcept
        -> Secp256k1Key = 0;
    virtual auto InstantiateSecp256k1Key(
        const Secret& privateKey,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Secp256k1::DefaultVersion) const noexcept
        -> Secp256k1Key = 0;
    virtual auto NewSecp256k1Key(
        const std::string& seedID,
        const Secret& seed,
        const opentxs::crypto::Bip32::Path& path,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Secp256k1::DefaultVersion) const
        -> Secp256k1Key = 0;
    virtual auto NewKey(
        const NymParameters& params,
        const PasswordPrompt& reason,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const VersionNumber version =
            opentxs::crypto::key::Asymmetric::DefaultVersion) const -> Key = 0;

    OPENTXS_NO_EXPORT virtual ~Asymmetric() = default;

protected:
    Asymmetric() = default;

private:
    Asymmetric(const Asymmetric&) = delete;
    Asymmetric(Asymmetric&&) = delete;
    auto operator=(const Asymmetric&) -> Asymmetric& = delete;
    auto operator=(Asymmetric&&) -> Asymmetric& = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif
