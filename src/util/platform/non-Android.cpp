// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/Log.hpp"     // IWYU pragma: associated

#include <iostream>

namespace opentxs::api::imp
{
auto Log::print(
    const int level,
    const UnallocatedCString& text,
    const UnallocatedCString& thread) noexcept -> void
{
    if (false == text.empty()) {
        std::cerr << "(" << thread << ") ";
        std::cerr << text << std::endl;
        std::cerr.flush();
    }
}
}  // namespace opentxs::api::imp
