// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "storage/tree/Root.hpp"  // IWYU pragma: associated

#include <ctime>
#include <functional>

#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/StorageRoot.pb.h"
#include "opentxs/protobuf/verify/StorageRoot.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"
#include "storage/tree/Tree.hpp"

#define CURRENT_VERSION 2

#define OT_METHOD "opentxs::storage::Root::"

namespace opentxs
{
namespace storage
{
Root::Root(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash,
    const std::int64_t interval,
    Flag& bucket)
    : ot_super(storage, hash)
    , gc_interval_(interval)
    , gc_root_()
    , current_bucket_(bucket)
    , gc_running_(Flag::Factory(false))
    , gc_resume_(Flag::Factory(false))
    , last_gc_()
    , sequence_()
    , gc_lock_()
    , gc_thread_()
    , tree_root_()
    , tree_lock_()
    , tree_()
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(CURRENT_VERSION);
    }
}

void Root::blank(const VersionNumber version)
{
    Node::blank(version);
    gc_root_ = Node::BLANK_HASH;
    current_bucket_.Off();
    last_gc_.store(static_cast<std::int64_t>(std::time(nullptr)));
    sequence_.store(0);
    tree_root_ = Node::BLANK_HASH;
}

void Root::cleanup() const
{
    Lock gclock(gc_lock_);

    if (gc_thread_) {
        if (gc_thread_->joinable()) { gc_thread_->join(); }

        gc_thread_.reset();
    }
}

void Root::collect_garbage(const opentxs::api::storage::Driver* to) const
{
    Lock lock(write_lock_);
    LogTrace(OT_METHOD)(__func__)(": Beginning garbage collection.").Flush();
    const auto resume = gc_resume_->Set(false);
    bool oldLocation = false;

    if (resume) {
        oldLocation = !current_bucket_;
    } else {
        gc_root_ = tree()->Root();
        oldLocation = current_bucket_.Toggle();
        save(lock);
        driver_.StoreRoot(true, root_);
    }

    lock.unlock();
    bool success{false};

    if (Node::check_hash(gc_root_)) {
        const storage::Tree tree(driver_, gc_root_);
        success = tree.Migrate(*to);
    }

    if (success) {
        driver_.EmptyBucket(oldLocation);
    } else {
        LogOutput(OT_METHOD)(__func__)(": Garbage collection failed. "
                                       "Will retry next cycle.")
            .Flush();
    }

    Lock gcLock(gc_lock_, std::defer_lock);
    std::lock(gcLock, lock);
    gc_running_->Off();
    gc_root_ = "";
    last_gc_.store(std::time(nullptr));
    save(lock);
    driver_.StoreRoot(true, root_);
    lock.unlock();
    gcLock.unlock();
    LogTrace(OT_METHOD)(__func__)(": Finished garbage collection.").Flush();
}

void Root::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageRoot> serialized;

    if (!driver_.LoadProto(hash, serialized)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to load root object file.")
            .Flush();
        OT_FAIL;
    }

    init_version(CURRENT_VERSION, *serialized);
    gc_root_ = normalize_hash(serialized->gcroot());
    current_bucket_.Set(serialized->altlocation());
    gc_running_->Off();
    gc_resume_->Set(serialized->gc());
    last_gc_.store(serialized->lastgc());
    sequence_.store(serialized->sequence());
    tree_root_ = normalize_hash(serialized->items());
}

auto Root::Migrate(const opentxs::api::storage::Driver& to) const -> bool
{
    if (0 == gc_interval_) {
        LogOutput(OT_METHOD)(__func__)(": Garbage collection disabled.")
            .Flush();

        return false;
    }

    const std::uint64_t time = std::time(nullptr);
    const bool intervalExceeded = ((time - last_gc_.load()) > gc_interval_);
    const bool resume = gc_resume_.get();
    const bool needToCollectGarbage = resume || intervalExceeded;

    if (needToCollectGarbage) {
        const auto running = gc_running_->Set(true);

        if (!running) {
            cleanup();
            gc_thread_.reset(
                new std::thread(&Root::collect_garbage, this, &to));

            return true;
        }
    }

    return false;
}

auto Root::mutable_Tree() -> Editor<storage::Tree>
{
    std::function<void(storage::Tree*, Lock&)> callback =
        [&](storage::Tree* in, Lock& lock) -> void { this->save(in, lock); };

    return Editor<storage::Tree>(write_lock_, tree(), callback);
}

auto Root::save(const Lock& lock, const opentxs::api::storage::Driver& to) const
    -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return to.StoreProto(serialized, root_);
}

auto Root::save(const Lock& lock) const -> bool
{
    OT_ASSERT(verify_write_lock(lock));

    sequence_++;

    return save(lock, driver_);
}

void Root::save(storage::Tree* tree, const Lock& lock)
{
    OT_ASSERT(verify_write_lock(lock));

    OT_ASSERT(nullptr != tree);

    Lock treeLock(tree_lock_);
    tree_root_ = tree->root_;
    treeLock.unlock();

    const bool saved = save(lock);

    OT_ASSERT(saved);
}

auto Root::Save(const opentxs::api::storage::Driver& to) const -> bool
{
    Lock lock(write_lock_);

    return save(lock, to);
}

auto Root::Sequence() const -> std::uint64_t { return sequence_.load(); }

auto Root::serialize() const -> proto::StorageRoot
{
    proto::StorageRoot output;
    output.set_version(version_);
    output.set_items(tree_root_);
    output.set_altlocation(current_bucket_);
    output.set_lastgc(last_gc_.load());
    output.set_gc(gc_running_.get());
    output.set_gcroot(gc_root_);
    output.set_sequence(sequence_);

    return output;
}

auto Root::tree() const -> storage::Tree*
{
    Lock lock(tree_lock_);

    if (!tree_) { tree_.reset(new storage::Tree(driver_, tree_root_)); }

    OT_ASSERT(tree_);

    lock.unlock();

    return tree_.get();
}

auto Root::Tree() const -> const storage::Tree& { return *tree(); }
}  // namespace storage
}  // namespace opentxs
