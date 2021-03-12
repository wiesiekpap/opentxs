// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "core/Flag.hpp"   // IWYU pragma: associated

#include <atomic>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Flag.hpp"

template class opentxs::Pimpl<opentxs::Flag>;

namespace opentxs
{
auto Flag::Factory(const bool state) -> OTFlag
{
    return OTFlag(new implementation::Flag(state));
}

namespace implementation
{
Flag::Flag(const bool state)
    : flag_(state)
{
}

auto Flag::Toggle() -> bool
{
    auto expected = flag_.load();

    while (false == flag_.compare_exchange_weak(expected, !expected)) { ; }

    return expected;
}
}  // namespace implementation
}  // namespace opentxs
