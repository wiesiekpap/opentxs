// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "1_Internal.hpp"     // IWYU pragma: associated
#include "api/Endpoints.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "internal/api/Factory.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"

#define ENDPOINT_VERSION_1 1

#define ACCOUNT_UPDATE_ENDPOINT "accountupdate"
#define BLOCKCHAIN_ACCOUNT_CREATED "blockchain/account/new"
#define BLOCKCHAIN_ASIO_ENDPOINT "blockchain/asio"
#define BLOCKCHAIN_BALANCE_ENDPOINT "blockchain/balance/interactive"
#define BLOCKCHAIN_BALANCE_PUBLISHER_ENDPOINT "blockchain/balance"
#define BLOCKCHAIN_BLOCK_QUEUE_UPDATED "blockchain/block/queue"
#define BLOCKCHAIN_BLOCK_UPDATED "blockchain/block/"
#define BLOCKCHAIN_FILTER_ENDPOINT "blockchain/filter"
#define BLOCKCHAIN_FILTER_INTERNAL "blockchain/filter/internal"
#define BLOCKCHAIN_MEMPOOL "blockchain/mempool"
#define BLOCKCHAIN_PEER_ACTIVE_ENDPOINT "blockchain/peer/active"
#define BLOCKCHAIN_PEER_CONNECTED_ENDPOINT "blockchain/peer/connected"
#define BLOCKCHAIN_REORG_ENDPOINT "blockchain/reorg"
#define BLOCKCHAIN_SCAN_ENDPOINT "blockchain/scan"
#define BLOCKCHAIN_STATE_ENDPOINT "blockchain/state"
#define BLOCKCHAIN_SYNC_DB_ENDPOINT "blockchain/sync/db"
#define BLOCKCHAIN_SYNC_ENDPOINT "blockchain/sync"
#define BLOCKCHAIN_TRANSACTIONS_ENDPOINT "blockchain/transactions"
#define CONNECTION_STATUS_ENDPOINT "connectionstatus"
#define CONTACT_UPDATE_ENDPOINT "contactupdate"
#define DHT_NYM_REQUEST_ENDPOINT "dht/requestnym"
#define DHT_SERVER_REQUEST_ENDPOINT "dht/requestserver"
#define DHT_UNIT_REQUEST_ENDPOINT "dht/requestunit"
#define FIND_NYM_ENDPOINT "otx/search/nym"
#define FIND_SERVER_ENDPOINT "otx/search/server"
#define FIND_UNIT_ENDPOINT "otx/search/unit"
#define INTERNAL_PROCESS_PUSH_NOTIFICATION_ENDPOINT "client/receivenotification"
#define INTERNAL_PUSH_NOTIFICATION_ENDPOINT "server/sendnotification"
#define ISSUER_UPDATE_ENDPOINT "issuerupdate"
#define MESSAGABILITY "otx/messagability"
#define MESSAGELOADED "otx/message_loaded"
#define NYM_CREATED_ENDPOINT "nymcreated"
#define NYM_UPDATE_ENDPOINT "nymupdate"
#define PAIR_EVENT_ENDPOINT "pairevent"
#define PEER_REPLY_UPDATE_ENDPOINT "peerreplyupdate"
#define PEER_REQUEST_UPDATE_ENDPOINT "peerrequestupdate"
#define PENDING_BAILMENT_ENDPOINT "peerrequest/pendingbailment"
#define SERVER_REPLY_RECEIVED_ENDPOINT "reply/received"
#define SERVER_REQUEST_SENT_ENDPOINT "request/sent"
#define SERVER_UPDATE_ENDPOINT "serverupdate"
#define SHUTDOWN "shutdown"
#define TASK_COMPLETE_ENDPOINT "taskcomplete/"
#define THREAD_UPDATE_ENDPOINT "threadupdate/"
#define UNIT_UPDATE_ENDPOINT "unitupdate"
#define WIDGET_UPDATE_ENDPOINT "ui/widgetupdate"
#define WORKFLOW_ACCOUNT_UPDATE_ENDPOINT "ui/workflowupdate/account"

//#define OT_METHOD "opentxs::api::implementation::Endpoints::"

namespace opentxs::factory
{
auto Endpoints(const network::zeromq::Context& zmq, const int instance) noexcept
    -> std::unique_ptr<api::Endpoints>
{
    using ReturnType = api::implementation::Endpoints;

    return std::make_unique<ReturnType>(zmq, instance);
}
}  // namespace opentxs::factory

namespace opentxs::api::implementation
{
Endpoints::Endpoints(
    const opentxs::network::zeromq::Context& zmq,
    const int instance) noexcept
    : zmq_(zmq)
    , instance_(instance)
{
}

auto Endpoints::build_inproc_path(const std::string& path, const int version)
    const noexcept -> std::string
{
    return zmq_.BuildEndpoint(path, instance_, version);
}

auto Endpoints::build_inproc_path(
    const std::string& path,
    const int version,
    const std::string& suffix) const noexcept -> std::string
{
    return zmq_.BuildEndpoint(path, instance_, version, suffix);
}

auto Endpoints::AccountUpdate() const noexcept -> std::string
{
    return build_inproc_path(ACCOUNT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainAccountCreated() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_ACCOUNT_CREATED, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainBalance() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_BALANCE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainBlockDownloadQueue() const noexcept -> std::string
{
    return build_inproc_path(
        BLOCKCHAIN_BLOCK_QUEUE_UPDATED, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainMempool() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_MEMPOOL, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainNewFilter() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_FILTER_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainPeer() const noexcept -> std::string
{
    return build_inproc_path(
        BLOCKCHAIN_PEER_ACTIVE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainPeerConnection() const noexcept -> std::string
{
    return build_inproc_path(
        BLOCKCHAIN_PEER_CONNECTED_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainReorg() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_REORG_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainScanProgress() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_SCAN_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainStateChange() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_STATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainSyncProgress() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_SYNC_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainSyncServerUpdated() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_SYNC_DB_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainTransactions() const noexcept -> std::string
{
    return build_inproc_path(
        BLOCKCHAIN_TRANSACTIONS_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainTransactions(
    const identifier::Nym& nym) const noexcept -> std::string
{
    auto path = std::string{BLOCKCHAIN_TRANSACTIONS_ENDPOINT} + '/' + nym.str();

    return build_inproc_path(path, ENDPOINT_VERSION_1);
}

auto Endpoints::BlockchainWalletUpdated() const noexcept -> std::string
{
    return build_inproc_path(
        BLOCKCHAIN_BALANCE_PUBLISHER_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::ConnectionStatus() const noexcept -> std::string
{
    return build_inproc_path(CONNECTION_STATUS_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::ContactUpdate() const noexcept -> std::string
{
    return build_inproc_path(CONTACT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::DhtRequestNym() const noexcept -> std::string
{
    return build_inproc_path(DHT_NYM_REQUEST_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::DhtRequestServer() const noexcept -> std::string
{
    return build_inproc_path(DHT_SERVER_REQUEST_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::DhtRequestUnit() const noexcept -> std::string
{
    return build_inproc_path(DHT_UNIT_REQUEST_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::FindNym() const noexcept -> std::string
{
    return build_inproc_path(FIND_NYM_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::FindServer() const noexcept -> std::string
{
    return build_inproc_path(FIND_SERVER_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::FindUnitDefinition() const noexcept -> std::string
{
    return build_inproc_path(FIND_UNIT_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::InternalBlockchainAsioContext() const noexcept -> std::string
{
    return build_inproc_path(BLOCKCHAIN_ASIO_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::InternalBlockchainBlockUpdated(
    const opentxs::blockchain::Type chain) const noexcept -> std::string
{
    auto path = std::string{BLOCKCHAIN_BLOCK_UPDATED} +
                std::to_string(static_cast<std::uint32_t>(chain));

    return build_inproc_path(path, ENDPOINT_VERSION_1);
}

auto Endpoints::InternalBlockchainFilterUpdated(
    const opentxs::blockchain::Type chain) const noexcept -> std::string
{
    auto path = std::string{BLOCKCHAIN_FILTER_INTERNAL} +
                std::to_string(static_cast<std::uint32_t>(chain));

    return build_inproc_path(path, ENDPOINT_VERSION_1);
}

auto Endpoints::InternalProcessPushNotification() const noexcept -> std::string
{
    return build_inproc_path(
        INTERNAL_PROCESS_PUSH_NOTIFICATION_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::InternalPushNotification() const noexcept -> std::string
{
    return build_inproc_path(
        INTERNAL_PUSH_NOTIFICATION_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::IssuerUpdate() const noexcept -> std::string
{
    return build_inproc_path(ISSUER_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::Messagability() const noexcept -> std::string
{
    return build_inproc_path(MESSAGABILITY, ENDPOINT_VERSION_1);
}

auto Endpoints::MessageLoaded() const noexcept -> std::string
{
    return build_inproc_path(MESSAGELOADED, ENDPOINT_VERSION_1);
}

auto Endpoints::NymCreated() const noexcept -> std::string
{
    return build_inproc_path(NYM_CREATED_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::NymDownload() const noexcept -> std::string
{
    return build_inproc_path(NYM_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::PairEvent() const noexcept -> std::string
{
    return build_inproc_path(PAIR_EVENT_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::PeerReplyUpdate() const noexcept -> std::string
{
    return build_inproc_path(PEER_REPLY_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::PeerRequestUpdate() const noexcept -> std::string
{
    return build_inproc_path(PEER_REQUEST_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::PendingBailment() const noexcept -> std::string
{
    return build_inproc_path(PENDING_BAILMENT_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::ServerReplyReceived() const noexcept -> std::string
{
    return build_inproc_path(
        SERVER_REPLY_RECEIVED_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::ServerRequestSent() const noexcept -> std::string
{
    return build_inproc_path(SERVER_REQUEST_SENT_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::ServerUpdate() const noexcept -> std::string
{
    return build_inproc_path(SERVER_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::Shutdown() const noexcept -> std::string
{
    return build_inproc_path(SHUTDOWN, ENDPOINT_VERSION_1);
}

auto Endpoints::TaskComplete() const noexcept -> std::string
{
    return build_inproc_path(TASK_COMPLETE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::ThreadUpdate(const std::string& thread) const noexcept
    -> std::string
{
    return build_inproc_path(
        THREAD_UPDATE_ENDPOINT, ENDPOINT_VERSION_1, thread);
}

auto Endpoints::UnitUpdate() const noexcept -> std::string
{
    return build_inproc_path(UNIT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::WidgetUpdate() const noexcept -> std::string
{
    return build_inproc_path(WIDGET_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}

auto Endpoints::WorkflowAccountUpdate() const noexcept -> std::string
{
    return build_inproc_path(
        WORKFLOW_ACCOUNT_UPDATE_ENDPOINT, ENDPOINT_VERSION_1);
}
}  // namespace opentxs::api::implementation
