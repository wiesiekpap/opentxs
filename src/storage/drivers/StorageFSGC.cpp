// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "storage/drivers/StorageFSGC.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <cassert>
#include <cstdio>
#include <memory>
#include <thread>

#include "2_Factory.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "storage/StorageConfig.hpp"

//#define OT_METHOD "opentxs::StorageFSGC::"

namespace opentxs
{
auto Factory::StorageFSGC(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket) -> opentxs::api::storage::Plugin*
{
    return new opentxs::storage::implementation::StorageFSGC(
        storage, config, hash, random, bucket);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageFSGC::StorageFSGC(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, config.path_, bucket)
{
    Init_StorageFSGC();
}

auto StorageFSGC::bucket_name(const bool bucket) const -> std::string
{
    return bucket ? config_.fs_secondary_bucket_ : config_.fs_primary_bucket_;
}

auto StorageFSGC::calculate_path(
    const std::string& key,
    const bool bucket,
    std::string& directory) const -> std::string
{
    directory = folder_ + path_seperator_ + bucket_name(bucket);

    return directory + path_seperator_ + key;
}

void StorageFSGC::Cleanup()
{
    Cleanup_StorageFSGC();
    ot_super::Cleanup();
}

void StorageFSGC::Cleanup_StorageFSGC()
{
    // future cleanup actions go here
}

auto StorageFSGC::EmptyBucket(const bool bucket) const -> bool
{
    assert(random_);

    std::string oldDirectory{};
    calculate_path("", bucket, oldDirectory);
    std::string random = random_();
    std::string newName = folder_ + path_seperator_ + random;

    if (0 != std::rename(oldDirectory.c_str(), newName.c_str())) {
        return false;
    }

    std::thread backgroundDelete(&StorageFSGC::purge, this, newName);
    backgroundDelete.detach();

    return boost::filesystem::create_directory(oldDirectory);
}

void StorageFSGC::Init_StorageFSGC()
{
    boost::filesystem::create_directory(
        folder_ + path_seperator_ + config_.fs_primary_bucket_);
    boost::filesystem::create_directory(
        folder_ + path_seperator_ + config_.fs_secondary_bucket_);
    ready_->On();
}

void StorageFSGC::purge(const std::string& path) const
{
    if (path.empty()) { return; }

    boost::filesystem::remove_all(path);
}

auto StorageFSGC::root_filename() const -> std::string
{
    OT_ASSERT(false == folder_.empty());
    OT_ASSERT(false == path_seperator_.empty());
    OT_ASSERT(false == config_.fs_root_file_.empty());

    return folder_ + path_seperator_ + config_.fs_root_file_;
}

StorageFSGC::~StorageFSGC() { Cleanup_StorageFSGC(); }
}  // namespace opentxs::storage::implementation
