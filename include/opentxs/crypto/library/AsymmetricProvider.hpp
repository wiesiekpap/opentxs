// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_ASYMMETRICPROVIDER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <optional>

#include "opentxs/Bytes.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs
{
namespace crypto
{
class OPENTXS_EXPORT AsymmetricProvider
{
public:
    static auto CurveToKeyType(const EcdsaCurve& curve)
        -> crypto::key::asymmetric::Algorithm;
    static auto KeyTypeToCurve(const crypto::key::asymmetric::Algorithm& type)
        -> EcdsaCurve;

    virtual auto SeedToCurveKey(
        const ReadView seed,
        const AllocateOutput privateKey,
        const AllocateOutput publicKey) const noexcept -> bool = 0;
    virtual auto SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept -> bool = 0;
    virtual auto RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const NymParameters& options = {},
        const AllocateOutput params = {}) const noexcept -> bool = 0;
    virtual auto Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        const AllocateOutput signature) const -> bool = 0;
    virtual auto SignContract(
        const api::Core& api,
        const String& contract,
        const ReadView key,
        const crypto::HashType hashType,
        Signature& output) const -> bool = 0;
    virtual auto Verify(
        const ReadView plaintext,
        const ReadView key,
        const ReadView signature,
        const crypto::HashType hashType) const -> bool = 0;
    virtual auto VerifyContractSignature(
        const api::Core& api,
        const String& strContractToVerify,
        const ReadView key,
        const Signature& theSignature,
        const crypto::HashType hashType) const -> bool = 0;

    OPENTXS_NO_EXPORT virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
