// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "util/Thread.hpp"  // IWYU pragma: associated

#include <Processthreadsapi.h>
#include <map>

#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

namespace opentxs
{
auto SetThisThreadsPriority(ThreadPriority priority) noexcept -> void
{
    static const auto map = std::map<ThreadPriority, int>{
        {ThreadPriority::Idle, THREAD_PRIORITY_IDLE},
        {ThreadPriority::Lowest, THREAD_PRIORITY_LOWEST},
        {ThreadPriority::BelowNormal, THREAD_PRIORITY_BELOW_NORMAL},
        {ThreadPriority::Normal, THREAD_PRIORITY_NORMAL},
        {ThreadPriority::AboveNormal, THREAD_PRIORITY_ABOVE_NORMAL},
        {ThreadPriority::Highest, THREAD_PRIORITY_HIGHEST},
        {ThreadPriority::TimeCritical, THREAD_PRIORITY_TIME_CRITICAL},
    };
    const auto value = map.at(priority);
    const auto handle = GetCurrentThread();
    const auto rc = SetThreadPriority(handle, value);

    if (false == rc) {
        LogOutput(__func__)(": failed to set thread priority to ")(
            opentxs::print(priority))
            .Flush();
    }
}
}  // namespace opentxs
