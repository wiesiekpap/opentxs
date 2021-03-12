// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: begin_exports
#include <lucre/bank.h>
// IWYU pragma: end_exports
#include <string>

#include "opentxs/Version.hpp"

namespace opentxs::blind
{
class LucreDumper
{
    std::string m_str_dumpfile;

public:
    LucreDumper();
    ~LucreDumper() = default;
};
}  // namespace opentxs::blind
