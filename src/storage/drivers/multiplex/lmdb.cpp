// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "storage/drivers/StorageMultiplex.hpp"  // IWYU pragma: associated

#include <memory>

#include "2_Factory.hpp"
#include "opentxs/api/storage/Plugin.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::storage::implementation::StorageMultiplex::"

namespace opentxs::storage::implementation
{
auto StorageMultiplex::init_lmdb(
    std::unique_ptr<opentxs::api::storage::Plugin>& plugin) -> void
{
    LogVerbose(OT_METHOD)(__func__)(": Initializing primary LMDB plugin.")
        .Flush();
    plugin.reset(Factory::StorageLMDB(
        storage_, config_, digest_, random_, primary_bucket_));
}
}  // namespace opentxs::storage::implementation
