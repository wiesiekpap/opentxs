// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/interface/ui/AccountList.hpp"
#include "opentxs/util/Container.hpp"
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
using AccountListList = List<
    AccountListExternalInterface,
    AccountListInternalInterface,
    AccountListRowID,
    AccountListRowInterface,
    AccountListRowInternal,
    AccountListRowBlank,
    AccountListSortKey,
    AccountListPrimaryID>;

class AccountList final : public AccountListList,
                          public Worker<api::session::Client>
{
public:
    AccountList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;

    ~AccountList() final;

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

    static auto print(Work type) noexcept -> const char*;

    UnallocatedSet<blockchain::Type> chains_;

    auto construct_row(
        const AccountListRowID& id,
        const AccountListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto subscribe(const blockchain::Type chain) const noexcept -> void;

    auto load_blockchain() noexcept -> void;
    auto load_blockchain_account(OTIdentifier&& id) noexcept -> void;
    auto load_blockchain_account(blockchain::Type chain) noexcept -> void;
    auto load_blockchain_account(
        OTIdentifier&& id,
        blockchain::Type chain) noexcept -> void;
    auto load_blockchain_account(
        OTIdentifier&& id,
        blockchain::Type chain,
        Amount&& balance) noexcept -> void;
    auto load_custodial() noexcept -> void;
    auto load_custodial_account(OTIdentifier&& id) noexcept -> void;
    auto load_custodial_account(OTIdentifier&& id, Amount&& balance) noexcept
        -> void;
    auto load_custodial_account(
        OTIdentifier&& id,
        OTUnitID&& contract,
        UnitType type,
        Amount&& balance,
        UnallocatedCString&& name) noexcept -> void;
    auto process_custodial(Message&& message) noexcept -> void;
    auto process_blockchain(Message&& message) noexcept -> void;
    auto process_blockchain_balance(Message&& message) noexcept -> void;
    auto startup() noexcept -> void;

    AccountList() = delete;
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    auto operator=(const AccountList&) -> AccountList& = delete;
    auto operator=(AccountList&&) -> AccountList& = delete;
};
}  // namespace opentxs::ui::implementation
