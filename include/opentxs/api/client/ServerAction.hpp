// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_SERVERACTION_HPP
#define OPENTXS_API_CLIENT_SERVERACTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace client
{
class ServerAction;
}  // namespace client

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace proto
{
class UnitDefinition;
}  // namespace proto

class OTPaymentPlan;
class OTSmartContract;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
class OPENTXS_EXPORT ServerAction
{
public:
    using Action = Pimpl<opentxs::client::ServerAction>;

    virtual auto ActivateSmartContract(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const std::string& agentName,
        std::unique_ptr<OTSmartContract>& contract) const -> Action = 0;
    virtual auto AdjustUsageCredits(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::Nym& targetNymID,
        const Amount adjustment) const -> Action = 0;
    virtual auto CancelPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const -> Action = 0;
    virtual auto CreateMarketOffer(
        const PasswordPrompt& reason,
        const Identifier& assetAccountID,
        const Identifier& currencyAccountID,
        const Amount scale,
        const Amount increment,
        const Amount quantity,
        const Amount price,
        const bool selling,
        const std::chrono::seconds lifetime,
        const std::string& stopSign,
        const Amount activationPrice) const -> Action = 0;
    virtual auto DepositPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        std::unique_ptr<OTPaymentPlan>& plan) const -> Action = 0;
    virtual auto DownloadMarketList(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const -> Action = 0;
    virtual auto DownloadMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& marketID,
        const Amount depth) const -> Action = 0;
    virtual auto DownloadMarketRecentTrades(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& marketID) const -> Action = 0;
    virtual auto DownloadNymMarketOffers(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const -> Action = 0;
    virtual auto ExchangeBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const Identifier& basketID,
        const bool direction) const -> Action = 0;
    virtual auto IssueBasketCurrency(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const proto::UnitDefinition& basket,
        const std::string& label = "") const -> Action = 0;
    virtual auto KillMarketOffer(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const -> Action = 0;
    virtual auto KillPaymentPlan(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const TransactionNumber number) const -> Action = 0;
    virtual auto PayDividend(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const Identifier& accountID,
        const std::string& memo,
        const Amount amountPerShare) const -> Action = 0;
    virtual auto TriggerClause(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const TransactionNumber transactionNumber,
        const std::string& clause,
        const std::string& parameter) const -> Action = 0;
    virtual auto UnregisterAccount(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID) const -> Action = 0;
    virtual auto UnregisterNym(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID) const -> Action = 0;
    virtual auto WithdrawVoucher(
        const PasswordPrompt& reason,
        const identifier::Nym& localNymID,
        const identifier::Server& serverID,
        const Identifier& accountID,
        const identifier::Nym& recipientNymID,
        const Amount amount,
        const std::string& memo) const -> Action = 0;

    OPENTXS_NO_EXPORT virtual ~ServerAction() = default;

protected:
    ServerAction() = default;

private:
    ServerAction(const ServerAction&) = delete;
    ServerAction(ServerAction&&) = delete;
    auto operator=(const ServerAction&) -> ServerAction& = delete;
    auto operator=(ServerAction&&) -> ServerAction& = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
