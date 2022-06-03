// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <mutex>
#include <shared_mutex>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/interface/ui/SeedTree.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class Amount;
class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using SeedTreeList = List<
    SeedTreeExternalInterface,
    SeedTreeInternalInterface,
    SeedTreeRowID,
    SeedTreeRowInterface,
    SeedTreeRowInternal,
    SeedTreeRowBlank,
    SeedTreeSortKey,
    SeedTreePrimaryID>;

class SeedTree final : public SeedTreeList, public Worker<api::session::Client>
{
public:
    auto ClearCallbacks() const noexcept -> void final;
    auto Debug() const noexcept -> UnallocatedCString final;
    auto DefaultNym() const noexcept -> OTNymID final;
    auto DefaultSeed() const noexcept -> OTIdentifier final;

    auto SetCallbacks(Callbacks&&) noexcept -> void final;

    SeedTree(
        const api::session::Client& api,
        const SimpleCallback& cb) noexcept;

    ~SeedTree() final;

protected:
    auto pipeline(Message&& in) noexcept -> void final;
    auto state_machine() noexcept -> bool final;

private:
    auto shut_down() noexcept -> void;

private:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        new_nym = value(WorkType::NymCreated),
        changed_nym = value(WorkType::NymUpdated),
        changed_seed = value(WorkType::SeedUpdated),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    using NymData = std::pair<SeedTreeItemSortKey, UnallocatedCString>;
    using NymMap = UnallocatedMap<OTNymID, NymData>;
    using SeedData =
        std::tuple<bool, UnallocatedCString, crypto::SeedStyle, NymMap>;
    using ChildMap = UnallocatedMap<OTIdentifier, SeedData>;
    using GuardedCallbacks =
        libguarded::ordered_guarded<Callbacks, std::shared_mutex>;
    using GuardedNym = libguarded::ordered_guarded<OTNymID, std::shared_mutex>;
    using GuardedSeed =
        libguarded::ordered_guarded<OTIdentifier, std::shared_mutex>;

    mutable GuardedCallbacks callbacks_;
    GuardedNym default_nym_;
    GuardedSeed default_seed_;

    auto construct_row(
        const SeedTreeRowID& id,
        const SeedTreeSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto load_seed(
        const Identifier& id,
        UnallocatedCString& name,
        crypto::SeedStyle& type,
        bool& isPrimary) const noexcept(false) -> void;
    auto load_nym(OTNymID&& id, ChildMap& out) const noexcept -> void;
    auto load_nyms(ChildMap& out) const noexcept -> void;
    auto load_seed(const Identifier& id, ChildMap& out) const noexcept(false)
        -> SeedData&;
    auto load_seeds(ChildMap& out) const noexcept -> void;
    auto nym_name(const identity::Nym& nym) const noexcept
        -> UnallocatedCString;

    auto add_children(ChildMap&& children) noexcept -> void;
    auto check_default_nym() noexcept -> void;
    auto check_default_seed() noexcept -> void;
    auto load() noexcept -> void;
    auto process_nym(Message&& in) noexcept -> void;
    auto process_nym(const identifier::Nym& id) noexcept -> void;
    auto process_seed(Message&& in) noexcept -> void;
    auto process_seed(const Identifier& id) noexcept -> void;
    auto startup() noexcept -> void;

    SeedTree() = delete;
    SeedTree(const SeedTree&) = delete;
    SeedTree(SeedTree&&) = delete;
    auto operator=(const SeedTree&) -> SeedTree& = delete;
    auto operator=(SeedTree&&) -> SeedTree& = delete;
};
}  // namespace opentxs::ui::implementation
