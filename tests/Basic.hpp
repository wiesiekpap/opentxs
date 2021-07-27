// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"

namespace ot = opentxs;

namespace ottest
{
auto Args(bool lowlevel = false) noexcept -> const ot::ArgList&;
auto Home() noexcept -> const std::string&;
auto WipeHome() noexcept -> void;
}  // namespace ottest
