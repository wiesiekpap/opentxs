// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "display/Definition.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/ui/AccountActivity.hpp"
#if OT_QT
#include "opentxs/ui/AmountValidator.hpp"
#include "opentxs/ui/DestinationValidator.hpp"
#include "opentxs/ui/DisplayScale.hpp"
#endif  // OT_QT
#include "opentxs/util/WorkType.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"
#include "util/Polarity.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
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

namespace proto
{
class PaymentEvent;
class PaymentWorkflow;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using AccountActivityList = List<
    AccountActivityExternalInterface,
    AccountActivityInternalInterface,
    AccountActivityRowID,
    AccountActivityRowInterface,
    AccountActivityRowInternal,
    AccountActivityRowBlank,
    AccountActivitySortKey,
    AccountActivityPrimaryID>;

/** Show the list of Workflows applicable to this account

    Each row is a BalanceItem which is associated with a Workflow state.

    Some Workflows will only have one entry in the AccountActivity based on
    their type, but others may have multiple entries corresponding to different
    states.
 */
class AccountActivity : public AccountActivityList,
                        protected Worker<AccountActivity>
{
public:
    const display::Definition scales_;
#if OT_QT
    DisplayScaleQt scales_qt_;
    AmountValidator amount_validator_;
    DestinationValidator destination_validator_;
#endif  // OT_QT

    auto AccountID() const noexcept -> std::string final
    {
        return account_id_->str();
    }
    auto Balance() const noexcept -> Amount final { return balance_.load(); }
    auto BalancePolarity() const noexcept -> int final
    {
        return polarity(balance_.load());
    }
    auto Contract() const noexcept -> const contract::Unit& final
    {
        return contract_.get();
    }
#if OT_BLOCKCHAIN
    auto DepositAddress() const noexcept -> std::string override
    {
        return DepositAddress(blockchain::Type::Unknown);
    }
    auto DepositAddress(const blockchain::Type) const noexcept
        -> std::string override
    {
        return {};
    }
    auto DepositChains() const noexcept
        -> std::vector<blockchain::Type> override
    {
        return {};
    }
#endif  // OT_BLOCKCHAIN
    auto Notary() const noexcept -> const contract::Server& final
    {
        return notary_.get();
    }
    auto Send(
        [[maybe_unused]] const Identifier& contact,
        [[maybe_unused]] const Amount amount,
        [[maybe_unused]] const std::string& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const Identifier& contact,
        [[maybe_unused]] const std::string& amount,
        [[maybe_unused]] const std::string& memo) const noexcept
        -> bool override
    {
        return false;
    }
#if OT_BLOCKCHAIN
    auto Send(
        [[maybe_unused]] const std::string& address,
        [[maybe_unused]] const Amount amount,
        [[maybe_unused]] const std::string& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto Send(
        [[maybe_unused]] const std::string& address,
        [[maybe_unused]] const std::string& amount,
        [[maybe_unused]] const std::string& memo) const noexcept
        -> bool override
    {
        return false;
    }
    auto SyncPercentage() const noexcept -> double override { return 100; }
    auto SyncProgress() const noexcept -> std::pair<int, int> override
    {
        return {1, 1};
    }
#endif  // OT_BLOCKCHAIN
    auto Type() const noexcept -> AccountType final { return type_; }
#if OT_BLOCKCHAIN
    auto ValidateAddress([[maybe_unused]] const std::string& text)
        const noexcept -> bool override
    {
        return false;
    }
#endif  // OT_BLOCKCHAIN
    auto ValidateAmount([[maybe_unused]] const std::string& text) const noexcept
        -> std::string override
    {
        return {};
    }

#if OT_BLOCKCHAIN
    using SyncCallback = std::function<void(int, int, double)>;

    virtual auto SetSyncCallback(const SyncCallback) noexcept -> void {}
#endif  // OT_BLOCKCHAIN

    ~AccountActivity() override;

protected:
    mutable std::atomic<Amount> balance_;
    const OTIdentifier account_id_;
    const AccountType type_;
    OTUnitDefinition contract_;
    OTServerContract notary_;

    using AccountActivityList::init;
    // NOTE only call in final class constructor bodies
    auto init(Endpoints endpoints) noexcept -> void;

    AccountActivity(
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const AccountType type,
        const SimpleCallback& cb,
        display::Definition&& scales) noexcept;

private:
    friend Worker<AccountActivity>;

    virtual auto startup() noexcept -> void = 0;

    auto construct_row(
        const AccountActivityRowID& id,
        const AccountActivitySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    virtual auto pipeline(const Message& in) noexcept -> void = 0;

    AccountActivity() = delete;
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    auto operator=(const AccountActivity&) -> AccountActivity& = delete;
    auto operator=(AccountActivity&&) -> AccountActivity& = delete;
};
}  // namespace opentxs::ui::implementation
