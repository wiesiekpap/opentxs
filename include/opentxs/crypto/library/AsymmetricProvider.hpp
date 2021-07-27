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
    static crypto::key::asymmetric::Algorithm CurveToKeyType(
        const EcdsaCurve& curve);
    static EcdsaCurve KeyTypeToCurve(
        const crypto::key::asymmetric::Algorithm& type);

    virtual bool SeedToCurveKey(
        const ReadView seed,
        const AllocateOutput privateKey,
        const AllocateOutput publicKey) const noexcept = 0;
    virtual bool SharedSecret(
        const ReadView publicKey,
        const ReadView privateKey,
        const SecretStyle style,
        Secret& secret) const noexcept = 0;
    virtual bool RandomKeypair(
        const AllocateOutput privateKey,
        const AllocateOutput publicKey,
        const opentxs::crypto::key::asymmetric::Role role =
            opentxs::crypto::key::asymmetric::Role::Sign,
        const NymParameters& options = {},
        const AllocateOutput params = {}) const noexcept = 0;
    virtual bool Sign(
        const ReadView plaintext,
        const ReadView key,
        const crypto::HashType hash,
        const AllocateOutput signature) const = 0;
    virtual bool SignContract(
        const api::Core& api,
        const String& contract,
        const ReadView key,
        const crypto::HashType hashType,
        Signature& output) const = 0;
    virtual bool Verify(
        const ReadView plaintext,
        const ReadView key,
        const ReadView signature,
        const crypto::HashType hashType) const = 0;
    virtual bool VerifyContractSignature(
        const api::Core& api,
        const String& strContractToVerify,
        const ReadView key,
        const Signature& theSignature,
        const crypto::HashType hashType) const = 0;

    OPENTXS_NO_EXPORT virtual ~AsymmetricProvider() = default;

protected:
    AsymmetricProvider() = default;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    AsymmetricProvider& operator=(const AsymmetricProvider&) = delete;
    AsymmetricProvider& operator=(AsymmetricProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
