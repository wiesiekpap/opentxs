// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/crypto/library/AsymmetricProvider.hpp"

namespace opentxs::crypto::implementation
{
class AsymmetricProviderNull final : virtual public crypto::AsymmetricProvider
{
public:
    auto RandomKeypair(
        const AllocateOutput,
        const AllocateOutput,
        const opentxs::crypto::key::asymmetric::Role,
        const NymParameters&,
        const AllocateOutput) const noexcept -> bool final
    {
        return false;
    }
    auto SeedToCurveKey(
        const ReadView,
        const AllocateOutput,
        const AllocateOutput) const noexcept -> bool final
    {
        return false;
    }
    auto SharedSecret(
        const ReadView,
        const ReadView,
        const SecretStyle,
        Secret&) const noexcept -> bool final
    {
        return false;
    }
    auto Sign(
        const ReadView,
        const ReadView,
        const crypto::HashType,
        const AllocateOutput) const -> bool final
    {
        return false;
    }
    auto SignContract(
        const api::Core&,
        const String&,
        const ReadView,
        const crypto::HashType,
        Signature&) const -> bool final
    {
        return false;
    }
    auto Verify(
        const ReadView,
        const ReadView,
        const ReadView,
        const crypto::HashType) const -> bool final
    {
        return false;
    }
    auto VerifyContractSignature(
        const api::Core&,
        const String&,
        const ReadView,
        const Signature&,
        const crypto::HashType) const -> bool final
    {
        return false;
    }

    AsymmetricProviderNull() = default;
    ~AsymmetricProviderNull() final = default;

private:
    AsymmetricProviderNull(const AsymmetricProviderNull&) = delete;
    AsymmetricProviderNull(AsymmetricProviderNull&&) = delete;
    auto operator=(const AsymmetricProviderNull&)
        -> AsymmetricProviderNull& = delete;
    auto operator=(AsymmetricProviderNull&&)
        -> AsymmetricProviderNull& = delete;
};
}  // namespace opentxs::crypto::implementation
