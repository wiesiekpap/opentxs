// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>

#include "opentxs/util/Container.hpp"

namespace opentxs
{
namespace core
{
enum class AddressType : std::uint8_t;
enum class UnitType : std::uint32_t;
}  // namespace core

auto print(core::UnitType) noexcept -> UnallocatedCString;
}  // namespace opentxs
