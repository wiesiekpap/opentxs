// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated

#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Seed.hpp"
#include "opentxs/util/Pimpl.hpp"  // IWYU pragma: keep

namespace std
{
auto less<opentxs::crypto::Seed>::operator()(
    const opentxs::crypto::Seed& lhs,
    const opentxs::crypto::Seed& rhs) const noexcept -> bool
{
    return lhs < rhs;
}

auto less<opentxs::Pimpl<opentxs::Data>>::operator()(
    const opentxs::OTData& lhs,
    const opentxs::OTData& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}

auto less<opentxs::OTIdentifier>::operator()(
    const opentxs::OTIdentifier& lhs,
    const opentxs::OTIdentifier& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}

auto less<opentxs::OTNotaryID>::operator()(
    const opentxs::OTNotaryID& lhs,
    const opentxs::OTNotaryID& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}

auto less<opentxs::OTNymID>::operator()(
    const opentxs::OTNymID& lhs,
    const opentxs::OTNymID& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}

auto less<opentxs::OTUnitID>::operator()(
    const opentxs::OTUnitID& lhs,
    const opentxs::OTUnitID& rhs) const -> bool
{
    return lhs.get() < rhs.get();
}
}  // namespace std
