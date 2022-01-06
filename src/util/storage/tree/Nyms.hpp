// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <tuple>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "serialization/protobuf/StorageNymList.pb.h"
#include "util/storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{
class Driver;
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

    static constexpr auto current_version_ = VersionNumber{3};

    mutable UnallocatedMap<UnallocatedCString, std::unique_ptr<storage::Nym>>
        nyms_;
    UnallocatedSet<UnallocatedCString> local_nyms_{};

    auto nym(const UnallocatedCString& id) const -> storage::Nym*;
    auto nym(const Lock& lock, const UnallocatedCString& id) const
        -> storage::Nym*;
    void save(
        storage::Nym* nym,
        const Lock& lock,
        const UnallocatedCString& id);

    void init(const UnallocatedCString& hash) final;
    auto save(const Lock& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    Nyms(const Driver& storage, const UnallocatedCString& hash);
    Nyms() = delete;
    Nyms(const Nyms&) = delete;
    Nyms(Nyms&&) = delete;
    auto operator=(const Nyms&) -> Nyms = delete;
    auto operator=(Nyms&&) -> Nyms = delete;

public:
    auto Exists(const UnallocatedCString& id) const -> bool;
    auto LocalNyms() const -> const UnallocatedSet<UnallocatedCString>;
    void Map(NymLambda lambda) const;
    auto Migrate(const Driver& to) const -> bool final;
    auto Nym(const UnallocatedCString& id) const -> const storage::Nym&;

    auto mutable_Nym(const UnallocatedCString& id) -> Editor<storage::Nym>;
    auto RelabelThread(
        const UnallocatedCString& threadID,
        const UnallocatedCString label) -> bool;
    void UpgradeLocalnym();

    ~Nyms() final = default;
};
}  // namespace storage
}  // namespace opentxs
