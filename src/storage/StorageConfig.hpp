// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>

#define OT_STORAGE_PRIMARY_PLUGIN_SQLITE "sqlite"
#define OT_STORAGE_PRIMARY_PLUGIN_LMDB "lmdb"
#define OT_STORAGE_PRIMARY_PLUGIN_MEMDB "mem"
#define OT_STORAGE_PRIMARY_PLUGIN_FS "fs"
#define STORAGE_CONFIG_PRIMARY_PLUGIN_KEY "primary_plugin"
#define STORAGE_CONFIG_FS_BACKUP_DIRECTORY_KEY "fs_backup_directory"
#define STORAGE_CONFIG_FS_ENCRYPTED_BACKUP_DIRECTORY_KEY "fs_encrypted_backup"

namespace C = std::chrono;

namespace opentxs
{

using InsertCB = std::function<void(const std::string&, const std::string&)>;

class StorageConfig
{
public:
    bool auto_publish_nyms_ = true;
    bool auto_publish_servers_ = true;
    bool auto_publish_units_ = true;
    std::int64_t gc_interval_ =
        C::duration_cast<C::seconds>(C::hours(1)).count();
    std::string path_{};
    InsertCB dht_callback_{};

#if OT_STORAGE_LMDB
    std::string primary_plugin_ = OT_STORAGE_PRIMARY_PLUGIN_LMDB;
#elif OT_STORAGE_SQLITE
    std::string primary_plugin_ = OT_STORAGE_PRIMARY_PLUGIN_SQLITE;
#elif OT_STORAGE_FS
    std::string primary_plugin_ = OT_STORAGE_PRIMARY_PLUGIN_FS;
#else
    std::string primary_plugin_{};
#endif

#ifdef OT_STORAGE_FS
    std::string fs_primary_bucket_ = "a";
    std::string fs_secondary_bucket_ = "b";
    std::string fs_root_file_ = "root";
    std::string fs_backup_directory_{""};
    std::string fs_encrypted_backup_directory_{""};
#endif

#ifdef OT_STORAGE_SQLITE
    std::string sqlite3_primary_bucket_ = "a";
    std::string sqlite3_secondary_bucket_ = "b";
    std::string sqlite3_control_table_ = "control";
    std::string sqlite3_root_key_ = "a";
    std::string sqlite3_db_file_ = "opentxs.sqlite3";
#endif

#ifdef OT_STORAGE_LMDB
    std::string lmdb_primary_bucket_ = "a";
    std::string lmdb_secondary_bucket_ = "b";
    std::string lmdb_control_table_ = "control";
    std::string lmdb_root_key_ = "root";
#endif
};
}  // namespace opentxs
