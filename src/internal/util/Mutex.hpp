// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <shared_mutex>

namespace opentxs
{
using Lock = std::unique_lock<std::mutex>;
using rLock = std::unique_lock<std::recursive_mutex>;
using sLock = std::shared_lock<std::shared_mutex>;
using eLock = std::unique_lock<std::shared_mutex>;
}  // namespace opentxs
