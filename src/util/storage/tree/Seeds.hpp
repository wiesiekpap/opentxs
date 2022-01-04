// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/StorageSeeds.pb.h"
#include "util/storage/tree/Node.hpp"

namespace opentxs
{
namespace proto
{
class Seed;
}  // namespace proto

namespace storage
{
class Driver;
class Tree;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Seeds final : public Node
{
private:
    friend Tree;

    static constexpr auto current_version_ = VersionNumber{2};

    std::string default_seed_;

    void init(const std::string& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    void set_default(
        const std::unique_lock<std::mutex>& lock,
        const std::string& id);
    auto serialize() const -> proto::StorageSeeds;

    Seeds(const Driver& storage, const std::string& hash);
    Seeds() = delete;
    Seeds(const Seeds&) = delete;
    Seeds(Seeds&&) = delete;
    auto operator=(const Seeds&) -> Seeds = delete;
    auto operator=(Seeds&&) -> Seeds = delete;

public:
    auto Alias(const std::string& id) const -> std::string;
    auto Default() const -> std::string;
    auto Load(
        const std::string& id,
        std::shared_ptr<proto::Seed>& output,
        std::string& alias,
        const bool checking) const -> bool;

    auto Delete(const std::string& id) -> bool;
    auto SetAlias(const std::string& id, const std::string& alias) -> bool;
    auto SetDefault(const std::string& id) -> bool;
    auto Store(const proto::Seed& data, const std::string& alias) -> bool;

    ~Seeds() final = default;
};
}  // namespace opentxs::storage
