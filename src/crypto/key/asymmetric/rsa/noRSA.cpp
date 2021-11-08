// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "internal/crypto/key/Factory.hpp"  // IWYU pragma: associated

#include "internal/crypto/key/Null.hpp"

namespace opentxs::factory
{
using ReturnType = crypto::key::blank::RSA;

auto RSAKey(
    const api::Session&,
    const crypto::AsymmetricProvider&,
    const proto::AsymmetricKey&) noexcept -> std::unique_ptr<crypto::key::RSA>
{
    return std::make_unique<ReturnType>();
}

auto RSAKey(
    const api::Session&,
    const crypto::AsymmetricProvider&,
    const crypto::key::asymmetric::Role,
    const VersionNumber,
    const NymParameters&,
    const opentxs::PasswordPrompt&) noexcept
    -> std::unique_ptr<crypto::key::RSA>
{
    return std::make_unique<ReturnType>();
}
}  // namespace opentxs::factory
