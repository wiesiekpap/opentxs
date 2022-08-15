// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::network
{
enum class ConnectionState : std::uint8_t {
    NOT_ESTABLISHED = 0,
    ACTIVE = 1,
    STALLED = 2
};
}  // namespace opentxs::network
