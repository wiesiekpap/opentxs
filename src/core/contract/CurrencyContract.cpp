// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "core/contract/CurrencyContract.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "Proto.hpp"
#include "core/contract/Unit.hpp"
#include "internal/core/Factory.hpp"
#include "internal/serialization/protobuf/Check.hpp"
#include "internal/serialization/protobuf/verify/UnitDefinition.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "serialization/protobuf/Signature.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"

namespace opentxs
{
auto Factory::CurrencyContract(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const opentxs::PasswordPrompt& reason,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement) noexcept
    -> std::shared_ptr<contract::unit::Currency>
{
    using ReturnType = contract::unit::implementation::Currency;

    auto output = std::make_shared<ReturnType>(
        api,
        nym,
        shortname,
        terms,
        unitOfAccount,
        version,
        displayDefinition,
        redemptionIncrement);

    if (false == bool(output)) { return {}; }

    auto& contract = *output;
    Lock lock(contract.lock_);

    if (contract.nym_) {
        auto serialized = contract.SigVersion(lock);
        auto sig = std::make_shared<proto::Signature>();

        if (!contract.update_signature(lock, reason)) { return {}; }
    }

    if (!contract.validate(lock)) { return {}; }

    return std::move(output);
}

auto Factory::CurrencyContract(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized) noexcept
    -> std::shared_ptr<contract::unit::Currency>
{
    using ReturnType = contract::unit::implementation::Currency;

    if (false == proto::Validate<ReturnType::SerializedType>(
                     serialized, VERBOSE, true)) {

        return {};
    }

    auto output = std::make_shared<ReturnType>(api, nym, serialized);

    if (false == bool(output)) { return {}; }

    auto& contract = *output;
    Lock lock(contract.lock_);

    if (!contract.validate(lock)) { return {}; }

    return std::move(output);
}
}  // namespace opentxs

namespace opentxs::contract::unit::implementation
{
Currency::Currency(
    const api::Session& api,
    const Nym_p& nym,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const opentxs::UnitType unitOfAccount,
    const VersionNumber version,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement)
    : Unit(
          api,
          nym,
          shortname,
          terms,
          unitOfAccount,
          version,
          displayDefinition,
          redemptionIncrement)
{
    Lock lock(lock_);
    first_time_init(lock);
}

Currency::Currency(
    const api::Session& api,
    const Nym_p& nym,
    const proto::UnitDefinition serialized)
    : Unit(api, nym, serialized)
{
    Lock lock(lock_);
    init_serialized(lock);
}

Currency::Currency(const Currency& rhs)
    : Unit(rhs)
{
}

auto Currency::IDVersion(const Lock& lock) const -> SerializedType
{
    auto contract = Unit::IDVersion(lock);

    return contract;
}
}  // namespace opentxs::contract::unit::implementation
