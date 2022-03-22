// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <tuple>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
using Endpoint = std::tuple<
    int,                 // address type
    int,                 // protocol version
    UnallocatedCString,  // hostname / address
    std::uint32_t,       // port
    VersionNumber>;
}  // namespace opentxs
