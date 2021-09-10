// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_WALLET_HPP
#define OPENTXS_API_WALLET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <ctime>
#include <list>
#include <memory>
#include <set>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/api/Editor.hpp"
#include "opentxs/blind/CashType.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/contract/basket/BasketContract.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Issuer;
}  // namespace client
}  // namespace api

namespace blind
{
class Purse;
}  // namespace blind

namespace otx
{
namespace context
{
class Base;
class Client;
class Server;
}  // namespace context
}  // namespace otx

namespace proto
{
class Credential;
class PeerReply;
class PeerRequest;
class ServerContract;
class UnitDefinition;
}  // namespace proto

class NymData;
class PeerObject;
}  // namespace opentxs

namespace opentxs
{
/** AccountInfo: accountID, nymID, serverID, unitID*/
using AccountInfo = std::tuple<OTIdentifier, OTNymID, OTServerID, OTUnitID>;

namespace api
{
/** \brief This class manages instantiated contracts and provides easy access
 *  to them.
 *
 * \ingroup native
 *
 *  It includes functionality which was previously found in OTWallet, and adds
 *  new capabilities such as the ability to (optionally) automatically perform
 *  remote lookups for contracts which are not already present in the local
 *  database.
 */
class OPENTXS_EXPORT Wallet
{
public:
    using AccountCallback = std::function<void(const Account&)>;

    virtual auto Account(const Identifier& accountID) const
        -> SharedAccount = 0;
    virtual auto AccountPartialMatch(const std::string& hint) const
        -> OTIdentifier = 0;
    virtual auto CreateAccount(
        const identifier::Nym& ownerNymID,
        const identifier::Server& notaryID,
        const identifier::UnitDefinition& instrumentDefinitionID,
        const identity::Nym& signer,
        Account::AccountType type,
        TransactionNumber stash,
        const PasswordPrompt& reason) const -> ExclusiveAccount = 0;
    virtual auto DeleteAccount(const Identifier& accountID) const -> bool = 0;
    virtual auto IssuerAccount(const identifier::UnitDefinition& unitID) const
        -> SharedAccount = 0;
    virtual auto mutable_Account(
        const Identifier& accountID,
        const PasswordPrompt& reason,
        const AccountCallback callback = nullptr) const -> ExclusiveAccount = 0;
    virtual auto UpdateAccount(
        const Identifier& accountID,
        const otx::context::Server&,
        const String& serialized,
        const PasswordPrompt& reason) const -> bool = 0;
    virtual auto UpdateAccount(
        const Identifier& accountID,
        const otx::context::Server&,
        const String& serialized,
        const std::string& label,
        const PasswordPrompt& reason) const -> bool = 0;
    [[deprecated]] virtual auto ImportAccount(
        std::unique_ptr<opentxs::Account>& imported) const -> bool = 0;

    /**   Load a read-only copy of a Context object
     *
     *    This method should only be called if the specific client or server
     *    version is not available (such as by classes common to client and
     *    server).
     *
     *    \param[in] notaryID
     *    \param[in] clientNymID
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID) const
        -> std::shared_ptr<const otx::context::Base> = 0;

    /**   Load a read-only copy of a ClientContext object
     *
     *    \param[in] remoteNymID context identifier (usually the other party's
     *                           nym id)
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto ClientContext(const identifier::Nym& remoteNymID) const
        -> std::shared_ptr<const otx::context::Client> = 0;

    /**   Load a read-only copy of a ServerContext object
     *
     *    \param[in] localNymID the identifier of the nym who owns the context
     *    \param[in] remoteID context identifier (usually the other party's nym
     *                       id)
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID) const
        -> std::shared_ptr<const otx::context::Server> = 0;

    /**   Load an existing Context object
     *
     *    This method should only be called if the specific client or server
     *    version is not available (such as by classes common to client and
     *    server).
     *
     *    WARNING: The context being loaded via this function must exist or else
     *    the function will assert.
     *
     *    \param[in] notaryID the identifier of the nym who owns the context
     *    \param[in] clientNymID context identifier (usually the other party's
     *                           nym id)
     */
    virtual auto mutable_Context(
        const identifier::Server& notaryID,
        const identifier::Nym& clientNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Base> = 0;

    /**   Load or create a ClientContext object
     *
     *    \param[in] remoteNymID context identifier (usually the other party's
     *                           nym id)
     */
    virtual auto mutable_ClientContext(
        const identifier::Nym& remoteNymID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Client> = 0;

    /**   Load or create a ServerContext object
     *
     *    \param[in] localNymID the identifier of the nym who owns the context
     *    \param[in] remoteID context identifier (usually the other party's nym
     *                        id)
     */
    virtual auto mutable_ServerContext(
        const identifier::Nym& localNymID,
        const Identifier& remoteID,
        const PasswordPrompt& reason) const -> Editor<otx::context::Server> = 0;

    /**   Returns a list of all issuers associated with a local nym */
    virtual auto IssuerList(const identifier::Nym& nymID) const
        -> std::set<OTNymID> = 0;

    /**   Load a read-only copy of an Issuer object
     *
     *    \param[in] nymID the identifier of the local nym
     *    \param[in] issuerID the identifier of the issuer nym
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    virtual auto Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const
        -> std::shared_ptr<const client::Issuer> = 0;

    /**   Load or create an Issuer object
     *
     *    \param[in] nymID the identifier of the local nym
     *    \param[in] issuerID the identifier of the issuer nym
     */
    virtual auto mutable_Issuer(
        const identifier::Nym& nymID,
        const identifier::Nym& issuerID) const -> Editor<client::Issuer> = 0;

    virtual auto IsLocalNym(const std::string& id) const -> bool = 0;

    virtual auto LocalNymCount() const -> std::size_t = 0;

    virtual auto LocalNyms() const -> std::set<OTNymID> = 0;

    /**   Obtain a smart pointer to an instantiated nym.
     *
     *    The smart pointer will not be initialized if the object does not
     *    exist or is invalid.
     *
     *    If the caller is willing to accept a network lookup delay, it can
     *    specify a timeout to be used in the event that the contract can not
     *    be located in local storage and must be queried from a remote
     *    location.
     *
     *    If no timeout is specified, the remote query will still happen in the
     *    background, but this method will return immediately with a null
     *    result.
     *
     *    \param[in] id the identifier of the nym to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                       willing to wait for a network lookup. The default
     *                       value of 0 will return immediately.
     */
    virtual auto Nym(
        const identifier::Nym& id,
        const std::chrono::milliseconds& timeout =
            std::chrono::milliseconds(0)) const -> Nym_p = 0;

    /**   Instantiate a nym from serialized form
     *
     *    The smart pointer will not be initialized if the provided serialized
     *    contract is invalid.
     *
     *    \param[in] nym the serialized version of the contract
     */
    virtual auto Nym(const identity::Nym::Serialized& nym) const -> Nym_p = 0;
    virtual auto Nym(const ReadView& bytes) const -> Nym_p = 0;

    virtual auto Nym(
        const PasswordPrompt& reason,
        const std::string name = "",
        const NymParameters& parameters = {},
        const contact::ContactItemType type =
            contact::ContactItemType::Individual) const -> Nym_p = 0;

    virtual auto mutable_Nym(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> NymData = 0;

    virtual auto Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const
        -> std::unique_ptr<const opentxs::NymFile> = 0;

    virtual auto mutable_Nymfile(
        const identifier::Nym& id,
        const PasswordPrompt& reason) const -> Editor<opentxs::NymFile> = 0;

    virtual auto NymByIDPartialMatch(const std::string& partialId) const
        -> Nym_p = 0;

    /**   Returns a list of all known nyms and their aliases
     */
    virtual auto NymList() const -> ObjectList = 0;

    virtual auto NymNameByIndex(const std::size_t index, String& name) const
        -> bool = 0;

    /**   Load a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which to retrive the peer object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    OPENTXS_NO_EXPORT virtual auto PeerReply(
        const identifier::Nym& nym,
        const Identifier& reply,
        const StorageBox& box,
        proto::PeerReply& serialized) const -> bool = 0;

    virtual auto PeerReply(
        const identifier::Nym& nym,
        const Identifier& reply,
        const StorageBox& box,
        AllocateOutput destination) const -> bool = 0;

    /**   Clean up the recipient's copy of a peer reply
     *
     *    The peer reply is moved from the nym's SentPeerReply
     *    box to the FinishedPeerReply box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] replyOrRequest the identifier of the peer reply object, or
     *               the id of its corresponding request
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerReplyComplete(
        const identifier::Nym& nym,
        const Identifier& replyOrRequest) const -> bool = 0;

    /**   Store the recipient's copy of a peer reply
     *
     *    The peer reply is stored in the SendPeerReply box for the
     *    specified nym.
     *
     *    The corresponding request is moved from the nym's IncomingPeerRequest
     *    box to the ProcessedPeerRequest box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the serialized peer reply object
     *    \returns true if the request is successfully stored
     */
    OPENTXS_NO_EXPORT virtual auto PeerReplyCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request,
        const proto::PeerReply& reply) const -> bool = 0;

    /**   Rollback a PeerReplyCreate call
     *
     *    The original request is returned to IncomingPeerRequest box
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the identifier of the peer reply object
     *    \returns true if the rollback is successful
     */
    virtual auto PeerReplyCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request,
        const Identifier& reply) const -> bool = 0;

    /**   Obtain a list of sent peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplySent(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of incoming peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplyIncoming(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of finished peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplyFinished(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of processed peer replies
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerReplyProcessed(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Store the senders's copy of a peer reply
     *
     *    The peer reply is stored in the IncomingPeerReply box for the
     *    specified nym.
     *
     *    The corresponding request is moved from the nym's SentPeerRequest
     *    box to the FinishedPeerRequest box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the corresponding request
     *    \param[in] reply the serialized peer reply object
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerReplyReceive(
        const identifier::Nym& nym,
        const PeerObject& reply) const -> bool = 0;

    /**   Load a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which to retrive the peer object
     *    \returns A smart pointer to the object. The smart pointer will not be
     *             instantiated if the object does not exist or is invalid.
     */
    OPENTXS_NO_EXPORT virtual auto PeerRequest(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time,
        proto::PeerRequest& serialized) const -> bool = 0;

    virtual auto PeerRequest(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box,
        std::time_t& time,
        AllocateOutput destination) const -> bool = 0;

    /**   Clean up the sender's copy of a peer reply
     *
     *    The peer reply is moved from the nym's IncomingPeerReply
     *    box to the ProcessedPeerReply box.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] reply the identifier of the peer reply object
     *    \returns true if the request is successfully moved
     */
    virtual auto PeerRequestComplete(
        const identifier::Nym& nym,
        const Identifier& reply) const -> bool = 0;

    /**   Store the initiator's copy of a peer request
     *
     *    The peer request is stored in the SentPeerRequest box for the
     *    specified nym.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the serialized peer request object
     *    \returns true if the request is successfully stored
     */
    OPENTXS_NO_EXPORT virtual auto PeerRequestCreate(
        const identifier::Nym& nym,
        const proto::PeerRequest& request) const -> bool = 0;

    /**   Rollback a PeerRequestCreate call
     *
     *    The request is deleted from to SentPeerRequest box
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer request
     *    \returns true if the rollback is successful
     */
    virtual auto PeerRequestCreateRollback(
        const identifier::Nym& nym,
        const Identifier& request) const -> bool = 0;

    /**   Delete a peer reply object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer reply object
     *    \param[in] box the box from which the peer object will be deleted
     */
    virtual auto PeerRequestDelete(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const -> bool = 0;

    /**   Obtain a list of sent peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestSent(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of incoming peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestIncoming(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of finished peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestFinished(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Obtain a list of processed peer requests
     *
     *    \param[in] nym the identifier of the nym whose box is returned
     */
    virtual auto PeerRequestProcessed(const identifier::Nym& nym) const
        -> ObjectList = 0;

    /**   Store the recipient's copy of a peer request
     *
     *    The peer request is stored in the IncomingPeerRequest box for the
     *    specified nym.
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the serialized peer request object
     *    \returns true if the request is successfully stored
     */
    virtual auto PeerRequestReceive(
        const identifier::Nym& nym,
        const PeerObject& request) const -> bool = 0;

    /**   Update the timestamp of a peer request object
     *
     *    \param[in] nym the identifier of the nym who owns the object
     *    \param[in] request the identifier of the peer request object
     *    \param[in] box the box from which the peer object will be deleted
     */
    virtual auto PeerRequestUpdate(
        const identifier::Nym& nym,
        const Identifier& request,
        const StorageBox& box) const -> bool = 0;

#if OT_CASH
    virtual auto Purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const bool checking = false) const
        -> std::unique_ptr<const blind::Purse> = 0;
    virtual auto mutable_Purse(
        const identifier::Nym& nym,
        const identifier::Server& server,
        const identifier::UnitDefinition& unit,
        const PasswordPrompt& reason,
        const blind::CashType = blind::CashType::Lucre) const
        -> Editor<blind::Purse> = 0;
#endif

    /**   Unload and delete a server contract
     *
     *    This method destroys the contract object, removes it from the
     *    in-memory map, and deletes it from local storage.
     *    \param[in]  id the indentifier of the contract to be removed
     *    \returns true if successful, false if the contract did not exist
     *
     */
    virtual auto RemoveServer(const identifier::Server& id) const -> bool = 0;

    /**   Unload and delete a unit definition contract
     *
     *    This method destroys the contract object, removes it from the
     *    in-memory map, and deletes it from local storage.
     *    \param[in]  id the indentifier of the contract to be removed
     *    \returns true if successful, false if the contract did not exist
     *
     */
    virtual auto RemoveUnitDefinition(
        const identifier::UnitDefinition& id) const -> bool = 0;

    /**   Obtain an instantiated server contract.
     *
     *    If the caller is willing to accept a network lookup delay, it can
     *    specify a timeout to be used in the event that the contract can not
     *    be located in local storage and must be queried from a remote
     *    location.
     *
     *    If no timeout is specified, the remote query will still happen in the
     *    background, but this method will return immediately with a null
     *    result.
     *
     *    \param[in] id the identifier of the contract to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                       willing to wait for a network lookup. The default
     *                       value of 0 will return immediately.
     *    \throw std::runtime_error the specified contract does not exist in the
     *                              wallet
     */
    virtual auto Server(
        const identifier::Server& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTServerContract = 0;

    /**   Instantiate a server contract from serialized form
     *
     *    \param[in] contract the serialized version of the contract
     *    \throw std::runtime_error the provided contract is not valid
     */
    OPENTXS_NO_EXPORT virtual auto Server(const proto::ServerContract& contract)
        const noexcept(false) -> OTServerContract = 0;
    virtual auto Server(const ReadView& contract) const noexcept(false)
        -> OTServerContract = 0;

    /**   Create a new server contract
     *
     *    \param[in] nymid the identifier of nym which will create the contract
     *    \param[in] name the official name of the server
     *    \param[in] terms human-readable server description & terms of use
     *    \param[in] url externally-reachable IP address or hostname
     *    \param[in] port externally-reachable listen port
     *    \throw std::runtime_error the contract can not be created
     */
    virtual auto Server(
        const std::string& nymid,
        const std::string& name,
        const std::string& terms,
        const std::list<contract::Server::Endpoint>& endpoints,
        const PasswordPrompt& reason,
        const VersionNumber version) const noexcept(false)
        -> OTServerContract = 0;

    /**   Returns a list of all available server contracts and their aliases
     */
    virtual auto ServerList() const -> ObjectList = 0;

    /**   Updates the alias for the specified nym.
     *
     *    An alias is a local label which is not part of the nym credentials
     *    itself.
     *
     *    \param[in] id the identifier of the nym whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified nym
     *    \returns true if successful, false if the nym can not be located
     */
    virtual auto SetNymAlias(
        const identifier::Nym& id,
        const std::string& alias) const -> bool = 0;

    /**   Updates the alias for the specified server contract.
     *
     *    An alias is a local label which is not part of the server contract
     *    itself.
     *
     *    \param[in] id the identifier of the contract whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified contract
     *    \returns true if successful, false if the contract can not be located
     */
    virtual auto SetServerAlias(
        const identifier::Server& id,
        const std::string& alias) const -> bool = 0;

    /**   Updates the alias for the specified unit definition contract.
     *
     *    An alias is a local label which is not part of the unit definition
     *    contract itself.
     *
     *    \param[in] id the identifier of the contract whose alias is to be set
     *    \param[in] alias the alias to set or update for the specified contract
     *    \returns true if successful, false if the contract can not be located
     */
    virtual auto SetUnitDefinitionAlias(
        const identifier::UnitDefinition& id,
        const std::string& alias) const -> bool = 0;

    /**   Obtain a list of all available unit definition contracts and their
     *    aliases
     */
    virtual auto UnitDefinitionList() const -> ObjectList = 0;

    /**   Obtain an instantiated unit definition contract.
     *
     *    If the caller is willing to accept a network lookup delay, it can
     *    specify a timeout to be used in the event that the contract can not
     *    be located in local storage and must be queried from a remote
     *    location.
     *
     *    If no timeout is specified, the remote query will still happen in the
     *    background, but this method will return immediately with a null
     *    result.
     *
     *    \param[in] id the identifier of the contract to be returned
     *    \param[in] timeout The caller can set a non-zero value here if it's
     *                     willing to wait for a network lookup. The default
     *                     value of 0 will return immediately.
     *    \throw std::runtime_error the specified contract does not exist in the
     *                              wallet
     */
    virtual auto UnitDefinition(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTUnitDefinition = 0;
    virtual auto BasketContract(
        const identifier::UnitDefinition& id,
        const std::chrono::milliseconds& timeout = std::chrono::milliseconds(
            0)) const noexcept(false) -> OTBasketContract = 0;

    /**   Instantiate a unit definition contract from serialized form
     *
     *    \param[in] contract the serialized version of the contract
     *    \throw std::runtime_error the provided contract is invalid
     */
    OPENTXS_NO_EXPORT virtual auto UnitDefinition(
        const proto::UnitDefinition& contract) const noexcept(false)
        -> OTUnitDefinition = 0;

    /**   Create a new currency contract
     *
     *    \param[in] nymid the identifier of nym which will create the contract
     *    \param[in] shortname a short human-readable identifier for the
     *                         contract
     *    \param[in] name the official name of the unit of account
     *    \param[in] symbol symbol for the unit of account
     *    \param[in] terms human-readable terms and conditions
     *    \param[in] tla three-letter acronym abbreviation of the unit of
     *                   account
     *    \param[in] power the number of decimal places to shift to display
     *                     fractional units
     *    \param[in] fraction the name of the fractional unit
     *    \throw std::runtime_error the contract can not be created
     */
    virtual auto UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const std::string& tla,
        const std::uint32_t power,
        const std::string& fraction,
        const contact::ContactItemType unitOfAccount,
        const PasswordPrompt& reason,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        noexcept(false) -> OTUnitDefinition = 0;

    /**   Create a new security contract
     *
     *    \param[in] nymid the identifier of nym which will create the contract
     *    \param[in] shortname a short human-readable identifier for the
     *                         contract
     *    \param[in] name the official name of the unit of account
     *    \param[in] symbol symbol for the unit of account
     *    \param[in] terms human-readable terms and conditions
     *    \throw std::runtime_error the contract can not be created
     */
    virtual auto UnitDefinition(
        const std::string& nymid,
        const std::string& shortname,
        const std::string& name,
        const std::string& symbol,
        const std::string& terms,
        const contact::ContactItemType unitOfAccount,
        const PasswordPrompt& reason,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        noexcept(false) -> OTUnitDefinition = 0;

    virtual auto CurrencyTypeBasedOnUnitType(
        const identifier::UnitDefinition& contractID) const
        -> contact::ContactItemType = 0;

    OPENTXS_NO_EXPORT virtual auto LoadCredential(
        const std::string& id,
        std::shared_ptr<proto::Credential>& credential) const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto SaveCredential(
        const proto::Credential& credential) const -> bool = 0;

    OPENTXS_NO_EXPORT virtual ~Wallet() = default;

protected:
    Wallet() = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
