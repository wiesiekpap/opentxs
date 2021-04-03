// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "storage/drivers/StorageMultiplex.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <iosfwd>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#include "2_Factory.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Plugin.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "storage/StorageConfig.hpp"
#include "storage/tree/Root.hpp"
#include "storage/tree/Tree.hpp"

#define OT_METHOD "opentxs::storage::implementation::StorageMultiplex::"

namespace opentxs::storage::implementation
{
auto StorageMultiplex::init_sqlite(
    std::unique_ptr<opentxs::api::storage::Plugin>& plugin) -> void
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(
        ": Initializing primary sqlite3 plugin.")
        .Flush();
    plugin.reset(Factory::StorageSqlite3(
        storage_, config_, digest_, random_, primary_bucket_));
}
}  // namespace opentxs::storage::implementation
