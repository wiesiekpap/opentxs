// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "storage/StorageConfig.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>

namespace C = std::chrono;

namespace opentxs
{
StorageConfig::StorageConfig() noexcept
    : auto_publish_nyms_(true)
    , auto_publish_servers_(true)
    , auto_publish_units_(true)
    , gc_interval_(C::duration_cast<C::seconds>(C::hours(1)).count())
    , path_()
    , dht_callback_()
    , primary_plugin_(default_plugin_)
    , fs_primary_bucket_("a")
    , fs_secondary_bucket_("b")
    , fs_root_file_("root")
    , fs_backup_directory_()
    , fs_encrypted_backup_directory_()
    , sqlite3_primary_bucket_("a")
    , sqlite3_secondary_bucket_("b")
    , sqlite3_control_table_("control")
    , sqlite3_root_key_("a")
    , sqlite3_db_file_("opentxs.sqlite3")
    , lmdb_primary_bucket_("a")
    , lmdb_secondary_bucket_("b")
    , lmdb_control_table_("control")
    , lmdb_root_key_("root")
{
}
}  // namespace opentxs
