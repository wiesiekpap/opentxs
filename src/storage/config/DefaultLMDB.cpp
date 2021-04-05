// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "storage/StorageConfig.hpp"  // IWYU pragma: associated

#include <memory>

namespace opentxs
{
const std::string StorageConfig::default_plugin_{
    OT_STORAGE_PRIMARY_PLUGIN_LMDB};
}  // namespace opentxs
