// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Endpoints final : public opentxs::api::Endpoints
{
public:
    auto AccountUpdate() const noexcept -> std::string final;
    auto BlockchainAccountCreated() const noexcept -> std::string final;
    auto BlockchainBalance() const noexcept -> std::string final;
    auto BlockchainBlockDownloadQueue() const noexcept -> std::string final;
    auto BlockchainMempool() const noexcept -> std::string final;
    auto BlockchainNewFilter() const noexcept -> std::string final;
    auto BlockchainPeer() const noexcept -> std::string final;
    auto BlockchainPeerConnection() const noexcept -> std::string final;
    auto BlockchainReorg() const noexcept -> std::string final;
    auto BlockchainScanProgress() const noexcept -> std::string final;
    auto BlockchainStateChange() const noexcept -> std::string final;
    auto BlockchainSyncProgress() const noexcept -> std::string final;
    auto BlockchainSyncServerUpdated() const noexcept -> std::string final;
    auto BlockchainTransactions() const noexcept -> std::string final;
    auto BlockchainTransactions(const identifier::Nym& nym) const noexcept
        -> std::string final;
    auto BlockchainWalletUpdated() const noexcept -> std::string final;
    auto ConnectionStatus() const noexcept -> std::string final;
    auto ContactUpdate() const noexcept -> std::string final;
    auto DhtRequestNym() const noexcept -> std::string final;
    auto DhtRequestServer() const noexcept -> std::string final;
    auto DhtRequestUnit() const noexcept -> std::string final;
    auto FindNym() const noexcept -> std::string final;
    auto FindServer() const noexcept -> std::string final;
    auto FindUnitDefinition() const noexcept -> std::string final;
    auto InternalBlockchainAsioContext() const noexcept -> std::string final;
    auto InternalBlockchainBlockUpdated(const opentxs::blockchain::Type chain)
        const noexcept -> std::string final;
    auto InternalBlockchainFilterUpdated(const opentxs::blockchain::Type chain)
        const noexcept -> std::string final;
    auto InternalProcessPushNotification() const noexcept -> std::string final;
    auto InternalPushNotification() const noexcept -> std::string final;
    auto IssuerUpdate() const noexcept -> std::string final;
    auto Messagability() const noexcept -> std::string final;
    auto MessageLoaded() const noexcept -> std::string final;
    auto NymCreated() const noexcept -> std::string final;
    auto NymDownload() const noexcept -> std::string final;
    auto PairEvent() const noexcept -> std::string final;
    auto PeerReplyUpdate() const noexcept -> std::string final;
    auto PeerRequestUpdate() const noexcept -> std::string final;
    auto PendingBailment() const noexcept -> std::string final;
    auto ServerReplyReceived() const noexcept -> std::string final;
    auto ServerRequestSent() const noexcept -> std::string final;
    auto ServerUpdate() const noexcept -> std::string final;
    auto Shutdown() const noexcept -> std::string final;
    auto TaskComplete() const noexcept -> std::string final;
    auto ThreadUpdate(const std::string& thread) const noexcept
        -> std::string final;
    auto UnitUpdate() const noexcept -> std::string final;
    auto WidgetUpdate() const noexcept -> std::string final;
    auto WorkflowAccountUpdate() const noexcept -> std::string final;

    Endpoints(
        const opentxs::network::zeromq::Context& zmq,
        const int instance) noexcept;

    ~Endpoints() final = default;

private:
    const opentxs::network::zeromq::Context& zmq_;
    const int instance_;

    auto build_inproc_path(const std::string& path, const int version)
        const noexcept -> std::string;
    auto build_inproc_path(
        const std::string& path,
        const int version,
        const std::string& suffix) const noexcept -> std::string;

    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    auto operator=(const Endpoints&) -> Endpoints& = delete;
    auto operator=(Endpoints&&) -> Endpoints& = delete;
};
}  // namespace opentxs::api::implementation
