// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_UTIL_COMMON_HPP
#define OPENTXS_CORE_UTIL_COMMON_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Types.hpp"

auto formatBool(bool in) -> std::string;
auto formatTimestamp(const opentxs::Time in) -> std::string;
auto getTimestamp() -> std::string;
auto parseTimestamp(std::string in) -> opentxs::Time;
#endif
