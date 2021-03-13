// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "storage/drivers/StorageMemDB.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "2_Factory.hpp"
#include "opentxs/core/Log.hpp"

//#define OT_METHOD "opentxs::StorageMemDB::"

namespace opentxs
{
auto Factory::StorageMemDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket) -> opentxs::api::storage::Plugin*
{
    return new opentxs::storage::implementation::StorageMemDB(
        storage, config, hash, random, bucket);
}
}  // namespace opentxs

namespace opentxs::storage::implementation
{
StorageMemDB::StorageMemDB(
    const api::storage::Storage& storage,
    const StorageConfig& config,
    const Digest& hash,
    const Random& random,
    const Flag& bucket)
    : ot_super(storage, config, hash, random, bucket)
    , root_("")
    , a_()
    , b_()
{
}

auto StorageMemDB::EmptyBucket(const bool bucket) const -> bool
{
    eLock lock(shared_lock_);

    if (bucket) {
        a_.clear();
    } else {
        b_.clear();
    }

    return true;
}

auto StorageMemDB::LoadFromBucket(
    const std::string& key,
    std::string& value,
    const bool bucket) const -> bool
{
    sLock lock(shared_lock_);

    try {
        if (bucket) {
            value = a_.at(key);
        } else {
            value = b_.at(key);
        }
    } catch (...) {

        return false;
    }

    return (false == value.empty());
}

auto StorageMemDB::LoadRoot() const -> std::string
{
    sLock lock(shared_lock_);

    return root_;
}

void StorageMemDB::store(
    [[maybe_unused]] const bool isTransaction,
    const std::string& key,
    const std::string& value,
    const bool bucket,
    std::promise<bool>* promise) const
{
    OT_ASSERT(nullptr != promise);

    if (bucket) {
        a_[key] = value;
    } else {
        b_[key] = value;
    }

    promise->set_value(true);
}

auto StorageMemDB::StoreRoot(
    [[maybe_unused]] const bool commit,
    const std::string& hash) const -> bool
{
    eLock lock(shared_lock_);
    root_ = hash;

    return true;
}
}  // namespace opentxs::storage::implementation
