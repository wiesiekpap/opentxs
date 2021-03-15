// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_ENDPOINTS_HPP
#define OPENTXS_API_ENDPOINTS_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/blockchain/Types.hpp"

#ifdef SWIG
// clang-format off
%rename(ZMQEndpoints) opentxs::api::Endpoints;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class Endpoints
{
public:
    /** Account balance update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  AccountUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string AccountUpdate() const noexcept = 0;

    /** Blockchain account creation notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainAccountCreated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainAccountCreated()
        const noexcept = 0;

    /** Blockchain balance notifications
     *
     *  A dealer socket can connect to this endpoint to send and receive
     *  BlockchainBalance tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainBalance() const noexcept = 0;

    /** Blockchain peer connection
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainPeerAdded tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainPeer() const noexcept = 0;

    /** Blockchain reorg and update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewHeader and BlockchainReorg tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainReorg() const noexcept = 0;

    /** Blockchain wallet scan progress
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainWalletScanProgress tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainScanProgress()
        const noexcept = 0;

    /** Blockchain enabled state change
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainStateChange tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainStateChange()
        const noexcept = 0;

    /** Blockchain wallet sync progress
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainSyncProgress tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainSyncProgress()
        const noexcept = 0;

    /** Blockchain transaction notifications (global)
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewTransaction tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainTransactions()
        const noexcept = 0;

    /** Blockchain transaction notifications (per-nym)
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  BlockchainNewTransaction tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string BlockchainTransactions(
        const identifier::Nym& nym) const noexcept = 0;

    /** Connection state notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  OTXConnectionStatus tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ConnectionStatus() const noexcept = 0;

    /** Contact account creation notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  ContactUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ContactUpdate() const noexcept = 0;

    /** Search for a nym in the DHT
     *
     *  A dealer socket can connect to this endpoint to send and receive
     *  DHTRequestNym tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string DhtRequestNym() const noexcept = 0;

    /** Search for a notary in the DHT
     *
     *  A dealer socket can connect to this endpoint to send and receive
     *  DHTRequestServer tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string DhtRequestServer() const noexcept = 0;

    /** Search for a unit definition in the DHT
     *
     *  A dealer socket can connect to this endpoint to send and receive
     *  DHTRequestUnit tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string DhtRequestUnit() const noexcept = 0;

    /** Search for a nym on known notaries
     *
     *  A push socket can connect to this endpoint to send
     *  OTXSearchNym tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string FindNym() const noexcept = 0;

    /** Search for a notary contract on known notaries
     *
     *  A push socket can connect to this endpoint to send
     *  OTXSearchServer tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string FindServer() const noexcept = 0;

    /** Search for a unit definition on known notaries
     *
     *  A push socket can connect to this endpoint to send
     *  OTXSearchUnit tagged messages.
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string FindUnitDefinition() const noexcept = 0;

    /** Communication between blockchain peers and boost::asio context
     *
     */
    virtual std::string InternalBlockchainAsioContext() const noexcept = 0;

    /** Notification of blockchain block filter updates
     *
     */
    virtual std::string InternalBlockchainBlockUpdated(
        const opentxs::blockchain::Type chain) const noexcept = 0;

    /** Notification of blockchain block filter updates
     *
     */
    virtual std::string InternalBlockchainFilterUpdated(
        const opentxs::blockchain::Type chain) const noexcept = 0;

    /** Push notification processing
     *
     *  This socket is for use by the Sync and ServerConnection classes only
     */
    virtual std::string InternalProcessPushNotification() const noexcept = 0;

    /** Push notification initiation
     *
     *  This socket is for use by the Server and MessageProcessor classes only
     */
    virtual std::string InternalPushNotification() const noexcept = 0;

    /** Issuer update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  IssuerUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string IssuerUpdate() const noexcept = 0;

    /** Nym created notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  NymCreated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string NymCreated() const noexcept = 0;

    /** Nym update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  NymUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string NymDownload() const noexcept = 0;

    /** Node pairing event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer message related to node pairing is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PairEvent message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PairEvent() const noexcept = 0;

    /** Peer reply event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer reply is received.
     *
     *  Messages bodies consist of two frame.
     *   * The first frame contains the recipient nym as a serialized string
     *   * The second frame contains a serialized proto::PeerReply message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PeerReplyUpdate() const noexcept = 0;

    /** Peer request event notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any peer request is received.
     *
     *  Messages bodies consist of one frame.
     *   * The first frame contains the recipient nym as a serialized string
     *   * The second frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PeerRequestUpdate() const noexcept = 0;

    /** Pending bailment notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  a pending bailment peer request has been received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains a serialized proto::PeerRequest message
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string PendingBailment() const noexcept = 0;

    /** Server reply notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any server reply is received.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ServerReplyReceived() const noexcept = 0;

    /** Server request notification
     *
     *  A subscribe socket can connect to this endpoint to be notified when
     *  any request message is sent to a notary.
     *
     *  Messages bodies consist of one frame.
     *   * The frame contains of the message type as a string
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ServerRequestSent() const noexcept = 0;

    /** Server contract update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  NotaryUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string ServerUpdate() const noexcept = 0;

    /** Notification of context shutdown
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  Shutdown tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string Shutdown() const noexcept = 0;

    /** Background task completion notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  OTXTaskComplete tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string TaskComplete() const noexcept = 0;

    /** Activity thread update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  ActivityThreadUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string ThreadUpdate(
        const std::string& thread) const noexcept = 0;

    /** Unit definition contract update notifications
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  UnitDefinitionUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for all session types.
     */
    OPENTXS_EXPORT virtual std::string UnitUpdate() const noexcept = 0;

    /** UI widget update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  UIModelUpdated tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string WidgetUpdate() const noexcept = 0;

    /** Account update notification
     *
     *  A subscribe socket can connect to this endpoint to receive
     *  WorkflowAccountUpdate tagged messages
     *
     *  See opentxs/util/WorkTypes.hpp for message format documentation
     *
     *  This endpoint is active for client sessions only.
     */
    OPENTXS_EXPORT virtual std::string WorkflowAccountUpdate()
        const noexcept = 0;

    OPENTXS_EXPORT virtual ~Endpoints() = default;

protected:
    Endpoints() = default;

private:
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    Endpoints& operator=(const Endpoints&) = delete;
    Endpoints& operator=(Endpoints&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
