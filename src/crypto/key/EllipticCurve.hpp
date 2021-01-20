// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "crypto/key/Asymmetric.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/protobuf/Enums.pb.h"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key

class EcdsaProvider;
}  // namespace crypto

namespace proto
{
class AsymmetricKey;
class Ciphertext;
class HDPath;
}  // namespace proto

class Data;
class OTPassword;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs::crypto::key::implementation
{
class EllipticCurve : virtual public key::EllipticCurve, public Asymmetric
{
public:
    operator bool() const noexcept final { return Asymmetric::operator bool(); }

    auto asPublic() const noexcept -> std::unique_ptr<key::Asymmetric> final
    {
        return asPublicEC();
    }
    auto asPublicEC() const noexcept
        -> std::unique_ptr<key::EllipticCurve> final;
    virtual auto CreateType() const -> NymParameterType = 0;
    auto Path() const noexcept -> const std::string override { return {}; }
    auto Path(proto::HDPath&) const noexcept -> bool override { return {}; }
    auto SignDER(
        const ReadView preimage,
        const proto::HashType hash,
        Space& output,
        const PasswordPrompt& reason) const noexcept -> bool final;

    virtual ~EllipticCurve() override = default;

protected:
    const crypto::EcdsaProvider& ecdsa_;

    static auto serialize_public(EllipticCurve* copy)
        -> std::shared_ptr<proto::AsymmetricKey>;

    virtual auto clone_ec() const -> EllipticCurve* = 0;
    virtual auto get_public() const
        -> std::shared_ptr<proto::AsymmetricKey> = 0;

    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serializedKey) noexcept(false);
    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKeyType keyType,
        const proto::KeyRole role,
        const VersionNumber version,
        const PasswordPrompt& reason) noexcept(false);
#if OT_CRYPTO_WITH_BIP32
    EllipticCurve(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKeyType keyType,
        const Secret& privateKey,
        const Data& publicKey,
        const proto::KeyRole role,
        const VersionNumber version,
        key::Symmetric& sessionKey,
        const PasswordPrompt& reason) noexcept(false);
#endif  // OT_CRYPTO_WITH_BIP32
    EllipticCurve(const EllipticCurve&) noexcept;

private:
    friend crypto::EcdsaProvider;

    static auto extract_key(
        const api::internal::Core& api,
        const crypto::EcdsaProvider& ecdsa,
        const proto::AsymmetricKey& serialized,
        Data& publicKey) -> std::unique_ptr<proto::Ciphertext>;

    EllipticCurve() = delete;
    EllipticCurve(EllipticCurve&&) = delete;
    auto operator=(const EllipticCurve&) -> EllipticCurve& = delete;
    auto operator=(EllipticCurve&&) -> EllipticCurve& = delete;
};
}  // namespace opentxs::crypto::key::implementation
