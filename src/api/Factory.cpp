// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "api/Factory.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/api/Factory.hpp"
#include "internal/core/Core.hpp"

// #define OT_METHOD "opentxs::api::implementation::Factory::"

namespace opentxs::factory
{
using ReturnType = api::implementation::Factory;

auto FactoryAPI(const api::Crypto& crypto) noexcept
    -> std::unique_ptr<api::Factory>
{
    return std::make_unique<ReturnType>(crypto);
}
}  // namespace opentxs::factory

namespace opentxs::api::implementation
{
Factory::Factory(const api::Crypto& crypto) noexcept
    : crypto_(crypto)
{
    [[maybe_unused]] const auto& notUsed = crypto_;  // TODO
}

auto Factory::Secret(const std::size_t bytes) const noexcept -> OTSecret
{
    return OTSecret{factory::Secret(bytes).release()};
}

auto Factory::SecretFromBytes(const ReadView bytes) const noexcept -> OTSecret
{
    return OTSecret{factory::Secret(bytes, true).release()};
}

auto Factory::SecretFromText(const std::string_view text) const noexcept
    -> OTSecret
{
    return OTSecret{factory::Secret(text, false).release()};
}
}  // namespace opentxs::api::implementation
