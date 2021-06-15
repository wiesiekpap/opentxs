// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "storage/drivers/StorageFS.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Plugin;
class Storage;
}  // namespace storage
}  // namespace api

class Factory;
class Flag;
class StorageConfig;
}  // namespace opentxs

namespace opentxs::storage::implementation
{
// Simple filesystem implementation of opentxs::storage
class StorageFSGC final : public StorageFS,
                          public virtual opentxs::api::storage::Driver
{
private:
    using ot_super = StorageFS;

public:
    auto EmptyBucket(const bool bucket) const -> bool final;

    void Cleanup() final;

    ~StorageFSGC() final;

private:
    friend Factory;

    auto bucket_name(const bool bucket) const -> std::string;
    auto calculate_path(
        const std::string& key,
        const bool bucket,
        std::string& directory) const -> std::string final;
    void purge(const std::string& path) const;
    auto root_filename() const -> std::string final;

    void Cleanup_StorageFSGC();
    void Init_StorageFSGC();

    StorageFSGC(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    StorageFSGC() = delete;
    StorageFSGC(const StorageFSGC&) = delete;
    StorageFSGC(StorageFSGC&&) = delete;
    auto operator=(const StorageFSGC&) -> StorageFSGC& = delete;
    auto operator=(StorageFSGC&&) -> StorageFSGC& = delete;
};
}  // namespace opentxs::storage::implementation
