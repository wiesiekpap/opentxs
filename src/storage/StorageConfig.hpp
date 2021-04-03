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

namespace opentxs
{
using InsertCB = std::function<void(const std::string&, const std::string&)>;

class StorageConfig
{
public:
    static const std::string default_plugin_;

    bool auto_publish_nyms_;
    bool auto_publish_servers_;
    bool auto_publish_units_;
    std::int64_t gc_interval_;
    std::string path_;
    InsertCB dht_callback_;

    std::string primary_plugin_;

    std::string fs_primary_bucket_;
    std::string fs_secondary_bucket_;
    std::string fs_root_file_;
    std::string fs_backup_directory_;
    std::string fs_encrypted_backup_directory_;

    std::string sqlite3_primary_bucket_;
    std::string sqlite3_secondary_bucket_;
    std::string sqlite3_control_table_;
    std::string sqlite3_root_key_;
    std::string sqlite3_db_file_;

    std::string lmdb_primary_bucket_;
    std::string lmdb_secondary_bucket_;
    std::string lmdb_control_table_;
    std::string lmdb_root_key_;

    StorageConfig() noexcept;
};
}  // namespace opentxs
