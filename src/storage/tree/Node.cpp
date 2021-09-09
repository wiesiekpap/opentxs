// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "storage/tree/Node.hpp"  // IWYU pragma: associated

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/protobuf/Contact.pb.h"
#include "opentxs/protobuf/Nym.pb.h"
#include "opentxs/protobuf/Seed.pb.h"
#include "opentxs/protobuf/StorageEnums.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"

#define OT_METHOD "opentxs::storage::Node::"

namespace opentxs::storage
{
const std::string Node::BLANK_HASH = "blankblankblankblankblank";

Node::Node(const opentxs::api::storage::Driver& storage, const std::string& key)
    : driver_(storage)
    , version_(0)
    , original_version_(0)
    , root_(key)
    , write_lock_()
    , item_map_()
{
}

void Node::blank(const VersionNumber version)
{
    version_ = version;
    original_version_ = version_;
    root_ = BLANK_HASH;
}

auto Node::check_hash(const std::string& hash) const -> bool
{
    const bool empty = hash.empty();
    const bool blank = (Node::BLANK_HASH == hash);

    return !(empty || blank);
}

auto Node::delete_item(const std::string& id) -> bool
{
    Lock lock(write_lock_);

    return delete_item(lock, id);
}

auto Node::delete_item(const Lock& lock, const std::string& id) -> bool
{
    OT_ASSERT(verify_write_lock(lock))

    const auto items = item_map_.erase(id);

    if (0 == items) { return false; }

    return save(lock);
}

auto Node::extract_revision(const proto::Contact& input) const -> std::uint64_t
{
    return input.revision();
}

auto Node::extract_revision(const proto::Nym& input) const -> std::uint64_t
{
    return input.revision();
}

auto Node::extract_revision(const proto::Seed& input) const -> std::uint64_t
{
    return input.index();
}

auto Node::get_alias(const std::string& id) const -> std::string
{
    std::string output;
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);

    if (item_map_.end() != it) { output = std::get<1>(it->second); }

    return output;
}

auto Node::List() const -> ObjectList
{
    ObjectList output;
    Lock lock(write_lock_);

    for (const auto& it : item_map_) {
        output.push_back({it.first, std::get<1>(it.second)});
    }

    lock.unlock();

    return output;
}

auto Node::load_raw(
    const std::string& id,
    std::string& output,
    std::string& alias,
    const bool checking) const -> bool
{
    std::lock_guard<std::mutex> lock(write_lock_);
    const auto& it = item_map_.find(id);
    const bool exists = (item_map_.end() != it);

    if (!exists) {
        if (!checking) {
            LogOutput(OT_METHOD)(__func__)(": Error: item with id ")(
                id)(" does not exist.")
                .Flush();
        }

        return false;
    }

    alias = std::get<1>(it->second);

    return driver_.Load(std::get<0>(it->second), checking, output);
}

auto Node::migrate(
    const std::string& hash,
    const opentxs::api::storage::Driver& to) const -> bool
{
    if (false == check_hash(hash)) { return true; }

    return driver_.Migrate(hash, to);
}

auto Node::Migrate(const opentxs::api::storage::Driver& to) const -> bool
{
    if (std::string(BLANK_HASH) == root_) {
        if (0 < item_map_.size()) {
            LogOutput(OT_METHOD)(__func__)(
                ": Items present in object with blank root hash.")
                .Flush();

            OT_FAIL;
        }

        return true;
    }

    bool output{true};
    output &= migrate(root_, to);

    for (const auto& item : item_map_) {
        const auto& hash = std::get<0>(item.second);
        output &= migrate(hash, to);
    }

    return output;
}

auto Node::normalize_hash(const std::string& hash) -> std::string
{
    if (hash.empty()) { return BLANK_HASH; }

    if (20 > hash.size()) {
        LogOutput(OT_METHOD)(__func__)(": Blanked out short hash ")(hash)(".")
            .Flush();

        return BLANK_HASH;
    }

    if (116 < hash.size()) {
        LogOutput(OT_METHOD)(__func__)(": Blanked out long hash ")(hash)(".")
            .Flush();

        return BLANK_HASH;
    }

    return hash;
}

auto Node::Root() const -> std::string
{
    Lock lock_(write_lock_);

    return root_;
}

void Node::serialize_index(
    const VersionNumber version,
    const std::string& id,
    const Metadata& metadata,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    set_hash(version, id, std::get<0>(metadata), output, type);
    output.set_alias(std::get<1>(metadata));
}

auto Node::set_alias(const std::string& id, const std::string& alias) -> bool
{
    std::string output;
    Lock lock(write_lock_);

    const bool exists = (item_map_.end() != item_map_.find(id));

    if (!exists) { return false; }

    std::get<1>(item_map_[id]) = alias;

    return save(lock);
}

void Node::set_hash(
    const VersionNumber version,
    const std::string& id,
    const std::string& hash,
    proto::StorageItemHash& output,
    const proto::StorageHashType type) const
{
    if (2 > version) {
        if (proto::STORAGEHASH_ERROR != type) {
            output.set_version(2);
        } else {
            output.set_version(version);
        }
    } else {
        output.set_version(version);
    }

    output.set_itemid(id);

    if (hash.empty()) {
        output.set_hash(Node::BLANK_HASH);
    } else {
        output.set_hash(hash);
    }

    output.set_type(type);
}

auto Node::store_raw(
    const std::string& data,
    const std::string& id,
    const std::string& alias) -> bool
{
    Lock lock(write_lock_);

    return store_raw(lock, data, id, alias);
}

auto Node::store_raw(
    const Lock& lock,
    const std::string& data,
    const std::string& id,
    const std::string& alias) -> bool
{
    OT_ASSERT(verify_write_lock(lock))

    auto& metadata = item_map_[id];
    auto& hash = std::get<0>(metadata);

    if (!driver_.Store(true, data, hash)) { return false; }

    if (!alias.empty()) { std::get<1>(metadata) = alias; }

    return save(lock);
}

auto Node::UpgradeLevel() const -> VersionNumber { return original_version_; }

auto Node::verify_write_lock(const Lock& lock) const -> bool
{
    if (lock.mutex() != &write_lock_) {
        LogOutput(OT_METHOD)(__func__)(": Incorrect mutex.").Flush();

        return false;
    }

    if (false == lock.owns_lock()) {
        LogOutput(OT_METHOD)(__func__)(": Lock not owned.").Flush();

        return false;
    }

    return true;
}
}  // namespace opentxs::storage
