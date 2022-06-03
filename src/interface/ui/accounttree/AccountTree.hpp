// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/AccountType.hpp"

#pragma once

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
#include "opentxs/interface/ui/AccountTree.hpp"
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
using AccountTreeList = List<
    AccountTreeExternalInterface,
    AccountTreeInternalInterface,
    AccountTreeRowID,
    AccountTreeRowInterface,
    AccountTreeRowInternal,
    AccountTreeRowBlank,
    AccountTreeSortKey,
    AccountTreePrimaryID>;

class AccountTree final : public AccountTreeList,
                          public Worker<api::session::Client>
{
public:
    auto Debug() const noexcept -> UnallocatedCString final;
    auto Owner() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }

    AccountTree(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;

    ~AccountTree() final;

protected:
    auto pipeline(Message&& in) noexcept -> void final;
    auto state_machine() noexcept -> bool final;

private:
    auto shut_down() noexcept -> void;

private:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        custodial = value(WorkType::AccountUpdated),
        blockchain = value(WorkType::BlockchainAccountCreated),
        balance = value(WorkType::BlockchainBalance),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    using SortIndex = int;
    using AccountName = UnallocatedCString;
    using AccountID = AccountCurrencyRowID;
    using CurrencyName = UnallocatedCString;
    using AccountData =
        std::tuple<SortIndex, AccountName, AccountType, CustomData>;
    using AccountMap = UnallocatedMap<AccountID, AccountData>;
    using CurrencyData =
        std::tuple<SortIndex, CurrencyName, CustomData, AccountMap>;
    using ChildMap = UnallocatedMap<UnitType, CurrencyData>;
    using SubscribeSet = UnallocatedSet<blockchain::Type>;

    auto construct_row(
        const AccountTreeRowID& id,
        const AccountTreeSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto load_blockchain(ChildMap& out, SubscribeSet& subscribe) const noexcept
        -> void;
    auto load_blockchain_account(
        OTIdentifier&& id,
        ChildMap& out,
        SubscribeSet& subscribe) const noexcept -> void;
    auto load_blockchain_account(
        blockchain::Type chain,
        ChildMap& out,
        SubscribeSet& subscribe) const noexcept -> void;
    auto load_blockchain_account(
        OTIdentifier&& id,
        blockchain::Type chain,
        ChildMap& out,
        SubscribeSet& subscribe) const noexcept -> void;
    auto load_blockchain_account(
        OTIdentifier&& id,
        blockchain::Type chain,
        Amount&& balance,
        ChildMap& out,
        SubscribeSet& subscribe) const noexcept -> void;
    auto load_custodial(ChildMap& out) const noexcept -> void;
    auto load_custodial_account(OTIdentifier&& id, ChildMap& out) const noexcept
        -> void;
    auto load_custodial_account(
        OTIdentifier&& id,
        Amount&& balance,
        ChildMap& out) const noexcept -> void;
    auto load_custodial_account(
        OTIdentifier&& id,
        OTUnitID&& contract,
        UnitType type,
        Amount&& balance,
        UnallocatedCString&& name,
        ChildMap& out) const noexcept -> void;
    auto subscribe(SubscribeSet&& chains) const noexcept -> void;

    auto add_children(ChildMap&& children) noexcept -> void;
    auto load() noexcept -> void;
    auto process_blockchain(Message&& message) noexcept -> void;
    auto process_blockchain_balance(Message&& message) noexcept -> void;
    auto process_custodial(Message&& message) noexcept -> void;
    auto startup() noexcept -> void;

    AccountTree() = delete;
    AccountTree(const AccountTree&) = delete;
    AccountTree(AccountTree&&) = delete;
    auto operator=(const AccountTree&) -> AccountTree& = delete;
    auto operator=(AccountTree&&) -> AccountTree& = delete;
};
}  // namespace opentxs::ui::implementation
