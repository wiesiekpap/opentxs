// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UTIL_WORKTYPE_HPP
#define OPENTXS_UTIL_WORKTYPE_HPP

#include <cstdint>

namespace opentxs
{
using OTZMQWorkType = std::uint16_t;

// NOTE 16384 - 32767 are reserved for client applications
enum class WorkType : OTZMQWorkType {
    Shutdown = 0,
    NymCreated = 1,
    NymUpdated = 2,
    NotaryUpdated = 3,
    UnitDefinitionUpdated = 4,
    ContactUpdated = 5,
    AccountUpdated = 6,
    IssuerUpdated = 7,
    ActivityThreadUpdated = 8,
    UIModelUpdated = 9,
    WorkflowAccountUpdate = 10,
    BlockchainAccountCreated = 11,
    BlockchainBalance = 12,
    BlockchainNewHeader = 13,
    BlockchainNewTransaction = 14,
    BlockchainPeerAdded = 15,
    BlockchainReorg = 16,
    BlockchainStateChange = 17,
    BlockchainSyncProgress = 18,
    OTXConnectionStatus = 19,
    OTXTaskComplete = 20,
    OTXSearchNym = 21,
    OTXSearchServer = 22,
    OTXSearchUnit = 23,
    DHTRequestNym = 24,
    DHTRequestServer = 25,
    DHTRequestUnit = 26,
    SyncRequest = 1024,
    SyncAcknowledgement = 1025,
    SyncReply = 1026,
    NewBlock = 1027,
};

constexpr auto value(const WorkType in) noexcept
{
    return static_cast<OTZMQWorkType>(in);
}

/*** Tagged Status Message Format
 *
 *   Messages using this taxonomy will always have the first body frame set to a
 *   WorkType value
 *
 *   Depending on the message type additional body frames may be present as
 *   described below
 *
 *   Shutdown: reports the pending shutdown of a client session or server
 *             session
 *
 *   NymCreated: reports the id of newly generated nyms
 *       * Additional frames:
 *          1: id as identifier::Nym (encoded as byte sequence)
 *
 *   NymUpdated: reports that (new or existing) nym has been modified
 *       * Additional frames:
 *          1: nym id as identifier::Nym (encoded as byte sequence)
 *
 *   NotaryUpdated: reports that a notary contract has been modified
 *       * Additional frames:
 *          1: id as identifier::Server (encoded as byte sequence)
 *
 *   UnitDefinitionUpdated: reports that a unit definition contract has been
 *                          modified
 *       * Additional frames:
 *          1: id as identifier::UnitDefinition (encoded as byte sequence)
 *
 *   ContactUpdated: reports that (new or existing) contact has been updated
 *       * Additional frames:
 *          1: contact id as Identifier (encoded as byte sequence)
 *
 *   AccountUpdated: reports that a custodial account has been modified
 *       * Additional frames:
 *          1: account id as Identifier (encoded as byte sequence)
 *          2: account balance as Amount
 *
 *   IssuerUpdated: reports that an issuer has been updated
 *       * Additional frames:
 *          1: local nym id as identifier::Nym (encoded as byte sequence)
 *          1: issuer id as identifier::Nym (encoded as byte sequence)
 *
 *   ActivityThreadUpdated: reports that an activity thread has been updated
 *       * Additional frames:
 *          1: thread id as Identifier (encoded as byte sequence)
 *
 *   UIModelUpdated: reports that a ui model has changed
 *       * Additional frames:
 *          1: widget id as Identifier (encoded as byte sequence)
 *
 *   WorkflowAccountUpdate: reports that a workflow has been modified
 *       * Additional frames:
 *          1: account id as Identifier (encoded as byte sequence)
 *
 *   BlockchainAccountCreated: reports the creation of a new blockchain account
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: account owner as identifier::Nym (encoded as byte sequence)
 *          3: account type as api::client::Blockchain::AccountType
 *          4: account id as Identifier (encoded as byte sequence)
 *
 *   BlockchainBalance: request and response messages for blockchain balance
 *                      information
 *       * Request message additional frames:
 *          1: chain type as blockchain::Type
 *          2: [optional] target nym as identifier::Nym (encoded as byte
 *                        sequence)
 *
 *       * Response message additional frames:
 *          1: chain type as blockchain::Type
 *          2: confirmed balance as Amount
 *          3: unconfirmed balance as Amount
 *          4: [optional] designated nym as identifier::Nym (encoded as byte
 *                        sequence)
 *
 *   BlockchainNewHeader: reports the receipt of a new blockchain header
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: new block hash (encoded as byte sequence)
 *          3: height of the new block as blockchain::block::Height
 *
 *   BlockchainNewTransaction: reports the receipt of a new blockchain
 *                             transaction
 *       * Additional frames:
 *          1: txid (encoded as byte sequence)
 *          2: chain type as blockchain::Type
 *
 *   BlockchainPeerAdded: reports when a new peer is connected for any active
 *                        blockchain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: peer address as string
 *
 *   BlockchainReorg: reports the receipt of a new blockchain header which
 *                    reorganizes the chain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: ancestor block hash (encoded as byte sequence)
 *          3: height of the ancestor block as blockchain::block::Height
 *
 *   BlockchainStateChange: reports changes to the enabled state of a
 *                          blockchain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: state as bool (enabled = true)
 *
 *   BlockchainSyncProgress: reports the wallet sync progress for a particular
 *                           blockchain
 *       * Additional frames:
 *          1: chain type as blockchain::Type
 *          2: current progress as blockchain::block::Height
 *          3: target height as blockchain::block::Height
 *
 *   OTXConnectionStatus: reports state changes to notary connections
 *       * Additional frames:
 *          1: notary id as identifier::Server (encoded as byte sequence)
 *          2: state as bool (active = true)
 *
 *   OTXTaskComplete: reports completion of OTX task
 *       * Additional frames:
 *          1: task is as api::client::OTX::TaskID
 *          2: resolution as bool (success = true)
 *
 *   DHTRequestNym: request and response messages for dht nym retrieval
 *       * Request message additional frames:
 *          1: target id as identifier::Nym (encoded as byte sequence)
 *
 *       * Response message additional frames:
 *          1: return value as bool (valid id = true)
 *
 *   OTXSearchNym: request messages for OTX nym search
 *       * Additional frames:
 *          1: target id as identifier::Nym (encoded as byte sequence)
 *
 *   OTXSearchServer: request messages for OTX notary contract search
 *       * Additional frames:
 *          1: target id as identifier::Server (encoded as byte sequence)
 *
 *   OTXSearchUnit: request messages for OTX unit definition contract search
 *       * Additional frames:
 *          1: target id as identifier::UnitDefinition (encoded as byte
 *             sequence)
 *
 *   DHTRequestServer: request and response messages for dht notary contract
 *                     retrieval
 *       * Request message additional frames:
 *          1: target id as identifier::Server (encoded as byte sequence)
 *
 *       * Response message additional frames:
 *          1: return value as bool (valid id = true)
 *
 *   DHTRequestUnit: request and response messages for dht unit definition
 *                   contract retrieval
 *       * Request message additional frames:
 *          1: target id as identifier::UnitDefinition (encoded as byte
 *             sequence)
 *
 *       * Response message additional frames:
 *          1: return value as bool (valid id = true)
 */
}  // namespace opentxs
#endif
