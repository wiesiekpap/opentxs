// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/protobuf/StorageRoot.pb.h"
#include "storage/tree/Node.hpp"
#include "storage/tree/Tree.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
namespace implementation
{
class Storage;
}  // namespace implementation

class Driver;
}  // namespace storage
}  // namespace api

namespace storage
{
namespace implementation
{
class StorageMultiplex;
}  // namespace implementation

class Root final : public Node
{
private:
    using ot_super = Node;
    friend opentxs::storage::implementation::StorageMultiplex;
    friend api::storage::implementation::Storage;

    const std::uint64_t gc_interval_{std::numeric_limits<std::int64_t>::max()};
    mutable std::string gc_root_;
    Flag& current_bucket_;
    mutable OTFlag gc_running_;
    mutable OTFlag gc_resume_;
    mutable std::atomic<std::uint64_t> last_gc_;
    mutable std::atomic<std::uint64_t> sequence_;
    mutable std::mutex gc_lock_;
    mutable std::unique_ptr<std::thread> gc_thread_;
    std::string tree_root_;
    mutable std::mutex tree_lock_;
    mutable std::unique_ptr<storage::Tree> tree_;

    auto serialize() const -> proto::StorageRoot;
    auto tree() const -> storage::Tree*;

    void blank(const VersionNumber version) final;
    void cleanup() const;
    void collect_garbage(const opentxs::api::storage::Driver* to) const;
    void init(const std::string& hash) final;
    auto save(const Lock& lock, const opentxs::api::storage::Driver& to) const
        -> bool;
    auto save(const Lock& lock) const -> bool final;
    void save(storage::Tree* tree, const Lock& lock);

    Root(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash,
        const std::int64_t interval,
        Flag& bucket);
    Root() = delete;
    Root(const Root&) = delete;
    Root(Root&&) = delete;
    auto operator=(const Root&) -> Root = delete;
    auto operator=(Root&&) -> Root = delete;

public:
    auto Tree() const -> const storage::Tree&;

    auto mutable_Tree() -> Editor<storage::Tree>;

    auto Migrate(const opentxs::api::storage::Driver& to) const -> bool final;
    auto Save(const opentxs::api::storage::Driver& to) const -> bool;
    auto Sequence() const -> std::uint64_t;

    ~Root() final = default;
};
}  // namespace storage
}  // namespace opentxs
