// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Proto.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace crypto
{
namespace key
{
class Asymmetric;
}  // namespace key
}  // namespace crypto

class PasswordPrompt;
class Signature;
class String;
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
class AsymmetricProvider : virtual public crypto::AsymmetricProvider
{
public:
    auto SeedToCurveKey(
        const ReadView seed,
        const AllocateOutput privateKey,
        const AllocateOutput publicKey) const noexcept -> bool final;
    auto SignContract(
        const api::Core& api,
        const String& contract,
        const ReadView key,
        const crypto::HashType hashType,
        Signature& output) const -> bool override;
    auto VerifyContractSignature(
        const api::Core& api,
        const String& strContractToVerify,
        const ReadView key,
        const Signature& theSignature,
        const crypto::HashType hashType) const -> bool override;

    ~AsymmetricProvider() override = default;

protected:
    AsymmetricProvider() noexcept;

private:
    AsymmetricProvider(const AsymmetricProvider&) = delete;
    AsymmetricProvider(AsymmetricProvider&&) = delete;
    auto operator=(const AsymmetricProvider&) -> AsymmetricProvider& = delete;
    auto operator=(AsymmetricProvider&&) -> AsymmetricProvider& = delete;
};
}  // namespace opentxs::crypto::implementation
