// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "util/Thread.hpp"  // IWYU pragma: associated

#include <pthread.h>
#include <mutex>

namespace opentxs
{

auto SetThisThreadsName(std::string_view threadName) noexcept -> void
{
    if (!threadName.empty()) {
        pthread_setname_np(pthread_self(), threadName.data());
    }
}
}  // namespace opentxs
