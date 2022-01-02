// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "otx/blind/lucre/Mint.hpp"  // IWYU pragma: associated

#include "internal/otx/blind/Factory.hpp"

namespace opentxs::factory
{
using ReturnType = otx::blind::Mint;
using Imp = ReturnType::Imp;

auto MintLucre(const api::Session& api) noexcept -> ReturnType
{
    return std::make_unique<Imp>(api).release();
}

auto MintLucre(
    const api::Session& api,
    const identifier::Notary&,
    const identifier::UnitDefinition&) noexcept -> ReturnType
{
    return std::make_unique<Imp>(api).release();
}

auto MintLucre(
    const api::Session& api,
    const identifier::Notary&,
    const identifier::Nym&,
    const identifier::UnitDefinition&) noexcept -> ReturnType
{
    return std::make_unique<Imp>(api).release();
}
}  // namespace opentxs::factory
