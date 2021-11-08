// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "storage/drivers/multiplex/Multiplex.hpp"  // IWYU pragma: associated

#include <memory>
#include <vector>

#include "internal/storage/drivers/Factory.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/storage/Plugin.hpp"
#include "opentxs/util/Log.hpp"

#define OT_METHOD "opentxs::storage::driver::Multiplex::"

namespace opentxs::storage::driver
{
auto Multiplex::init_fs(std::unique_ptr<storage::Plugin>& plugin) -> void
{
    LogVerbose()(OT_METHOD)(__func__)(
        ": Initializing primary filesystem plugin.")
        .Flush();
    plugin = factory::StorageFSGC(
        crypto_, asio_, storage_, config_, primary_bucket_);
}

auto Multiplex::init_fs_backup(const std::string& dir) -> void
{
    backup_plugins_.emplace_back(factory::StorageFSArchive(
        crypto_, asio_, storage_, config_, primary_bucket_, dir, null_));
}
}  // namespace opentxs::storage::driver
