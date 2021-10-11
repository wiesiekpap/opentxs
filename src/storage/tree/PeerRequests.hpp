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
#include "opentxs/protobuf/StorageNymList.pb.h"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace proto
{
class PeerRequest;
}  // namespace proto

namespace storage
{
class Driver;
class Nym;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class PeerRequests final : public Node
{
private:
    friend Nym;

    void init(const std::string& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    PeerRequests(const Driver& storage, const std::string& hash);
    PeerRequests() = delete;
    PeerRequests(const PeerRequests&) = delete;
    PeerRequests(PeerRequests&&) = delete;
    auto operator=(const PeerRequests&) -> PeerRequests = delete;
    auto operator=(PeerRequests&&) -> PeerRequests = delete;

public:
    auto Load(
        const std::string& id,
        std::shared_ptr<proto::PeerRequest>& output,
        std::string& alias,
        const bool checking) const -> bool;

    auto Delete(const std::string& id) -> bool;
    auto SetAlias(const std::string& id, const std::string& alias) -> bool;
    auto Store(const proto::PeerRequest& data, const std::string& alias)
        -> bool;

    ~PeerRequests() final = default;
};
}  // namespace opentxs::storage
