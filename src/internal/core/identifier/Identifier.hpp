// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "util/Blank.hpp"

namespace opentxs
{
template <>
struct make_blank<OTNymID> {
    static auto value(const api::Core&) -> OTNymID
    {
        return identifier::Nym::Factory();
    }
};

template <>
struct make_blank<OTServerID> {
    static auto value(const api::Core&) -> OTServerID
    {
        return identifier::Server::Factory();
    }
};

template <>
struct make_blank<OTUnitID> {
    static auto value(const api::Core&) -> OTUnitID
    {
        return identifier::UnitDefinition::Factory();
    }
};
}  // namespace opentxs
