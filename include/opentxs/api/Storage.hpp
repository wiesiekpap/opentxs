// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_STORAGE_STORAGE_HPP
#define OPENTXS_API_STORAGE_STORAGE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <memory>
#include <set>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/api/client/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"

namespace opentxs
{
namespace proto
{
class Bip47Channel;
class Ciphertext;
class Contact;
class Context;
class Credential;
class HDAccount;
class Issuer;
class Nym;
class PaymentWorkflow;
class PeerReply;
class PeerRequest;
class Purse;
class Seed;
class ServerContract;
class StorageThread;
class UnitDefinition;
}  // namespace proto

using NymLambda = std::function<void(const proto::Nym&)>;
using ServerLambda = std::function<void(const proto::ServerContract&)>;
using UnitLambda = std::function<void(const proto::UnitDefinition&)>;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace storage
{
class Storage
{
public:
    using Bip47ChannelList = std::set<OTIdentifier>;

    virtual auto AccountAlias(const Identifier& accountID) const
        -> std::string = 0;
    virtual auto AccountList() const -> ObjectList = 0;
    virtual auto AccountContract(const Identifier& accountID) const
        -> OTUnitID = 0;
    virtual auto AccountIssuer(const Identifier& accountID) const
        -> OTNymID = 0;
    virtual auto AccountOwner(const Identifier& accountID) const -> OTNymID = 0;
    virtual auto AccountServer(const Identifier& accountID) const
        -> OTServerID = 0;
    virtual auto AccountSigner(const Identifier& accountID) const
        -> OTNymID = 0;
    virtual auto AccountUnit(const Identifier& accountID) const
        -> contact::ContactItemType = 0;
    virtual auto AccountsByContract(const identifier::UnitDefinition& contract)
        const -> std::set<OTIdentifier> = 0;
    virtual auto AccountsByIssuer(const identifier::Nym& issuerNym) const
        -> std::set<OTIdentifier> = 0;
    virtual auto AccountsByOwner(const identifier::Nym& ownerNym) const
        -> std::set<OTIdentifier> = 0;
    virtual auto AccountsByServer(const identifier::Server& server) const
        -> std::set<OTIdentifier> = 0;
    virtual auto AccountsByUnit(const contact::ContactItemType unit) const
        -> std::set<OTIdentifier> = 0;
    virtual auto Bip47Chain(
        const identifier::Nym& nymID,
        const Identifier& channelID) const -> contact::ContactItemType = 0;
    virtual auto Bip47ChannelsByChain(
        const identifier::Nym& nymID,
        const contact::ContactItemType chain) const -> Bip47ChannelList = 0;
    virtual auto BlockchainAccountList(
        const std::string& nymID,
        const contact::ContactItemType type) const -> std::set<std::string> = 0;
    virtual auto BlockchainAccountType(
        const std::string& nymID,
        const std::string& accountID) const -> contact::ContactItemType = 0;
    virtual auto BlockchainThreadMap(
        const identifier::Nym& nym,
        const Data& txid) const noexcept -> std::vector<OTIdentifier> = 0;
    virtual auto BlockchainTransactionList(
        const identifier::Nym& nym) const noexcept -> std::vector<OTData> = 0;
    virtual auto CheckTokenSpent(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const std::string& key) const -> bool = 0;
    virtual auto ContactAlias(const std::string& id) const -> std::string = 0;
    virtual auto ContactList() const -> ObjectList = 0;
    virtual auto ContextList(const std::string& nymID) const -> ObjectList = 0;
    virtual auto ContactOwnerNym(const std::string& nymID) const
        -> std::string = 0;
    virtual void ContactSaveIndices() const = 0;
    virtual auto ContactUpgradeLevel() const -> VersionNumber = 0;
    virtual auto CreateThread(
        const std::string& nymID,
        const std::string& threadID,
        const std::set<std::string>& participants) const -> bool = 0;
    virtual auto DeleteAccount(const std::string& id) const -> bool = 0;
    virtual auto DefaultSeed() const -> std::string = 0;
    virtual auto DeleteContact(const std::string& id) const -> bool = 0;
    virtual auto DeletePaymentWorkflow(
        const std::string& nymID,
        const std::string& workflowID) const -> bool = 0;
    virtual auto HashType() const -> std::uint32_t = 0;
    virtual auto IssuerList(const std::string& nymID) const -> ObjectList = 0;
    virtual auto Load(
        const std::string& accountID,
        std::string& output,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymID,
        const std::string& accountID,
        proto::HDAccount& output,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nymID,
        const Identifier& channelID,
        proto::Bip47Channel& output,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Contact& contact,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Contact& contact,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nym,
        const std::string& id,
        proto::Context& context,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Credential& cred,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Nym& nym,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Nym& nym,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        AllocateOutput destination,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymID,
        const std::string& id,
        proto::Issuer& issuer,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymID,
        const std::string& workflowID,
        proto::PaymentWorkflow& workflow,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        std::string& output,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        proto::PeerReply& request,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box,
        proto::PeerRequest& request,
        std::time_t& time,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const identifier::Nym& nym,
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        proto::Purse& output,
        const bool checking) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Seed& seed,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::Seed& seed,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::ServerContract& contract,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::ServerContract& contract,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& nymId,
        const std::string& threadId,
        proto::StorageThread& thread) const -> bool = 0;
    virtual auto Load(proto::Ciphertext& output, const bool checking = false)
        const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::UnitDefinition& contract,
        const bool checking = false) const -> bool = 0;
    virtual auto Load(
        const std::string& id,
        proto::UnitDefinition& contract,
        std::string& alias,
        const bool checking = false) const -> bool = 0;
    virtual auto LocalNyms() const -> const std::set<std::string> = 0;
    virtual void MapPublicNyms(NymLambda& lambda) const = 0;
    virtual void MapServers(ServerLambda& lambda) const = 0;
    virtual void MapUnitDefinitions(UnitLambda& lambda) const = 0;
    virtual auto MarkTokenSpent(
        const identifier::Server& notary,
        const identifier::UnitDefinition& unit,
        const std::uint64_t series,
        const std::string& key) const -> bool = 0;
    virtual auto MoveThreadItem(
        const std::string& nymId,
        const std::string& fromThreadID,
        const std::string& toThreadID,
        const std::string& itemID) const -> bool = 0;
    virtual auto NymBoxList(const std::string& nymID, const StorageBox box)
        const -> ObjectList = 0;
    virtual auto NymList() const -> ObjectList = 0;
    virtual auto PaymentWorkflowList(const std::string& nymID) const
        -> ObjectList = 0;
    virtual auto PaymentWorkflowLookup(
        const std::string& nymID,
        const std::string& sourceID) const -> std::string = 0;
    virtual auto PaymentWorkflowsByAccount(
        const std::string& nymID,
        const std::string& accountID) const -> std::set<std::string> = 0;
    virtual auto PaymentWorkflowsByState(
        const std::string& nymID,
        const api::client::PaymentWorkflowType type,
        const api::client::PaymentWorkflowState state) const
        -> std::set<std::string> = 0;
    virtual auto PaymentWorkflowsByUnit(
        const std::string& nymID,
        const std::string& unitID) const -> std::set<std::string> = 0;
    virtual auto PaymentWorkflowState(
        const std::string& nymID,
        const std::string& workflowID) const
        -> std::pair<
            api::client::PaymentWorkflowType,
            api::client::PaymentWorkflowState> = 0;
    virtual auto RelabelThread(
        const std::string& threadID,
        const std::string& label) const -> bool = 0;
    virtual auto RemoveBlockchainThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const opentxs::blockchain::Type chain,
        const Data& txid) const noexcept -> bool = 0;
    virtual auto RemoveNymBoxItem(
        const std::string& nymID,
        const StorageBox box,
        const std::string& itemID) const -> bool = 0;
    virtual auto RemoveServer(const std::string& id) const -> bool = 0;
    virtual auto RemoveThreadItem(
        const identifier::Nym& nym,
        const Identifier& thread,
        const std::string& id) const -> bool = 0;
    virtual auto RemoveUnitDefinition(const std::string& id) const -> bool = 0;
    virtual auto RenameThread(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& newID) const -> bool = 0;
    virtual void RunGC() const = 0;
    virtual auto ServerAlias(const std::string& id) const -> std::string = 0;
    virtual auto ServerList() const -> ObjectList = 0;
    virtual auto SeedList() const -> ObjectList = 0;
    virtual auto SetAccountAlias(
        const std::string& id,
        const std::string& alias) const -> bool = 0;
    virtual auto SetContactAlias(
        const std::string& id,
        const std::string& alias) const -> bool = 0;
    virtual auto SetDefaultSeed(const std::string& id) const -> bool = 0;
    virtual auto SetNymAlias(const std::string& id, const std::string& alias)
        const -> bool = 0;
    virtual auto SetPeerRequestTime(
        const std::string& nymID,
        const std::string& id,
        const StorageBox box) const -> bool = 0;
    virtual auto SetReadState(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& itemId,
        const bool unread) const -> bool = 0;
    virtual auto SetSeedAlias(const std::string& id, const std::string& alias)
        const -> bool = 0;
    virtual auto SetServerAlias(const std::string& id, const std::string& alias)
        const -> bool = 0;
    virtual auto SetThreadAlias(
        const std::string& nymId,
        const std::string& threadId,
        const std::string& alias) const -> bool = 0;
    virtual auto SetUnitDefinitionAlias(
        const std::string& id,
        const std::string& alias) const -> bool = 0;
    virtual auto Store(
        const std::string& accountID,
        const std::string& data,
        const std::string& alias,
        const identifier::Nym& ownerNym,
        const identifier::Nym& signerNym,
        const identifier::Nym& issuerNym,
        const identifier::Server& server,
        const identifier::UnitDefinition& contract,
        const contact::ContactItemType unit) const -> bool = 0;
    virtual auto Store(
        const std::string& nymID,
        const contact::ContactItemType type,
        const proto::HDAccount& data) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nymID,
        const Identifier& channelID,
        const proto::Bip47Channel& data) const -> bool = 0;
    virtual auto Store(const proto::Contact& data) const -> bool = 0;
    virtual auto Store(const proto::Context& data) const -> bool = 0;
    virtual auto Store(const proto::Credential& data) const -> bool = 0;
    virtual auto Store(
        const proto::Nym& data,
        const std::string& alias = std::string("")) const -> bool = 0;
    virtual auto Store(
        const ReadView& data,
        const std::string& alias = std::string("")) const -> bool = 0;
    virtual auto Store(const std::string& nymID, const proto::Issuer& data)
        const -> bool = 0;
    virtual auto Store(
        const std::string& nymID,
        const proto::PaymentWorkflow& data) const -> bool = 0;
    virtual auto Store(
        const std::string& nymid,
        const std::string& threadid,
        const std::string& itemid,
        const std::uint64_t time,
        const std::string& alias,
        const std::string& data,
        const StorageBox box,
        const std::string& account = std::string("")) const -> bool = 0;
    virtual auto Store(
        const identifier::Nym& nym,
        const Identifier& thread,
        const opentxs::blockchain::Type chain,
        const Data& txid,
        const Time time) const noexcept -> bool = 0;
    virtual auto Store(
        const proto::PeerReply& data,
        const std::string& nymid,
        const StorageBox box) const -> bool = 0;
    virtual auto Store(
        const proto::PeerRequest& data,
        const std::string& nymid,
        const StorageBox box) const -> bool = 0;
    virtual auto Store(const identifier::Nym& nym, const proto::Purse& purse)
        const -> bool = 0;
    virtual auto Store(
        const proto::Seed& data,
        const std::string& alias = std::string("")) const -> bool = 0;
    virtual auto Store(
        const proto::ServerContract& data,
        const std::string& alias = std::string("")) const -> bool = 0;
    virtual auto Store(const proto::Ciphertext& serialized) const -> bool = 0;
    virtual auto Store(
        const proto::UnitDefinition& data,
        const std::string& alias = std::string("")) const -> bool = 0;
    virtual auto ThreadList(const std::string& nymID, const bool unreadOnly)
        const -> ObjectList = 0;
    virtual auto ThreadAlias(
        const std::string& nymID,
        const std::string& threadID) const -> std::string = 0;
    virtual auto UnaffiliatedBlockchainTransaction(
        const identifier::Nym& recipient,
        const Data& txid) const noexcept -> bool = 0;
    virtual auto UnitDefinitionAlias(const std::string& id) const
        -> std::string = 0;
    virtual auto UnitDefinitionList() const -> ObjectList = 0;
    virtual auto UnreadCount(
        const std::string& nymId,
        const std::string& threadId) const -> std::size_t = 0;
    virtual void UpgradeNyms() = 0;

    virtual ~Storage() = default;

protected:
    Storage() = default;

private:
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;
};
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif
