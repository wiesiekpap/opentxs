// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "Proto.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/protobuf/StorageServers.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Driver;
}  // namespace storage
}  // namespace api

namespace proto
{
class ServerContract;
}  // namespace proto

namespace storage
{
class Tree;
}  // namespace storage
}  // namespace opentxs

namespace opentxs
{
namespace storage
{
class Servers final : public Node
{
private:
    friend Tree;

    void init(const std::string& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageServers;

    Servers(
        const opentxs::api::storage::Driver& storage,
        const std::string& hash);
    Servers() = delete;
    Servers(const Servers&) = delete;
    Servers(Servers&&) = delete;
    auto operator=(const Servers&) -> Servers = delete;
    auto operator=(Servers&&) -> Servers = delete;

public:
    auto Alias(const std::string& id) const -> std::string;
    auto Load(
        const std::string& id,
        std::shared_ptr<proto::ServerContract>& output,
        std::string& alias,
        const bool checking) const -> bool;
    void Map(ServerLambda lambda) const;

    auto Delete(const std::string& id) -> bool;
    auto SetAlias(const std::string& id, const std::string& alias) -> bool;
    auto Store(
        const proto::ServerContract& data,
        const std::string& alias,
        std::string& plaintext) -> bool;

    ~Servers() final = default;
};
}  // namespace storage
}  // namespace opentxs
