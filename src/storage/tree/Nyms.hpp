// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <tuple>

#include "Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/protobuf/StorageNymList.pb.h"
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

namespace storage
{
class Nym;
class Tree;
}  // namespace storage
}  // namespace opentxs

namespace opentxs
{
namespace storage
{
class Nyms final : public Node
{
private:
    friend Tree;

    mutable std::map<std::string, std::unique_ptr<storage::Nym>> nyms_;
    std::set<std::string> local_nyms_{};

    auto nym(const std::string& id) const -> storage::Nym*;
    auto nym(const Lock& lock, const std::string& id) const -> storage::Nym*;
    void save(storage::Nym* nym, const Lock& lock, const std::string& id);

    void init(const std::string& hash) final;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    Nyms(const opentxs::api::storage::Driver& storage, const std::string& hash);
    Nyms() = delete;
    Nyms(const Nyms&) = delete;
    Nyms(Nyms&&) = delete;
    auto operator=(const Nyms&) -> Nyms = delete;
    auto operator=(Nyms&&) -> Nyms = delete;

public:
    auto Exists(const std::string& id) const -> bool;
    auto LocalNyms() const -> const std::set<std::string>;
    void Map(NymLambda lambda) const;
    auto Migrate(const opentxs::api::storage::Driver& to) const -> bool final;
    auto Nym(const std::string& id) const -> const storage::Nym&;

    auto mutable_Nym(const std::string& id) -> Editor<storage::Nym>;
    auto RelabelThread(const std::string& threadID, const std::string label)
        -> bool;
    void UpgradeLocalnym();

    ~Nyms() final = default;
};
}  // namespace storage
}  // namespace opentxs
