// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/util/Bytes.hpp"

namespace opentxs::network::zeromq
{
OPENTXS_EXPORT auto MakeArbitraryInproc() noexcept -> std::string;
auto MakeDeterministicInproc(
    const std::string& path,
    const int instance,
    const int version) noexcept -> std::string;
auto MakeDeterministicInproc(
    const std::string& path,
    const int instance,
    const int version,
    const std::string& suffix) noexcept -> std::string;
auto RawToZ85(const ReadView input, const AllocateOutput output) noexcept
    -> bool;
auto Z85ToRaw(const ReadView input, const AllocateOutput output) noexcept
    -> bool;
}  // namespace opentxs::network::zeromq
