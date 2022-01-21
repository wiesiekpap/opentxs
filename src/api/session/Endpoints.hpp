// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <robin_hood.h>

#include "internal/api/session/Endpoints.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Endpoints;
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
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::api::session::imp
{
class Endpoints final : public internal::Endpoints
{
public:
    auto AccountUpdate() const noexcept -> UnallocatedCString final;
    auto BlockchainAccountCreated() const noexcept -> UnallocatedCString final;
    auto BlockchainBalance() const noexcept -> UnallocatedCString final;
    auto BlockchainBlockAvailable() const noexcept -> UnallocatedCString final;
    auto BlockchainBlockDownloadQueue() const noexcept
        -> UnallocatedCString final;
    auto BlockchainBlockUpdated(const opentxs::blockchain::Type chain)
        const noexcept -> UnallocatedCString final;
    auto BlockchainFilterUpdated(const opentxs::blockchain::Type chain)
        const noexcept -> UnallocatedCString final;
    auto BlockchainMempool() const noexcept -> UnallocatedCString final;
    auto BlockchainNewFilter() const noexcept -> UnallocatedCString final;
    auto BlockchainPeer() const noexcept -> UnallocatedCString final;
    auto BlockchainPeerConnection() const noexcept -> UnallocatedCString final;
    auto BlockchainReorg() const noexcept -> UnallocatedCString final;
    auto BlockchainScanProgress() const noexcept -> UnallocatedCString final;
    auto BlockchainStateChange() const noexcept -> UnallocatedCString final;
    auto BlockchainSyncProgress() const noexcept -> UnallocatedCString final;
    auto BlockchainSyncServerUpdated() const noexcept
        -> UnallocatedCString final;
    auto BlockchainTransactions() const noexcept -> UnallocatedCString final;
    auto BlockchainTransactions(const identifier::Nym& nym) const noexcept
        -> UnallocatedCString final;
    auto BlockchainWalletUpdated() const noexcept -> UnallocatedCString final;
    auto ConnectionStatus() const noexcept -> UnallocatedCString final;
    auto ContactUpdate() const noexcept -> UnallocatedCString final;
    auto DhtRequestNym() const noexcept -> UnallocatedCString final;
    auto DhtRequestServer() const noexcept -> UnallocatedCString final;
    auto DhtRequestUnit() const noexcept -> UnallocatedCString final;
    auto FindNym() const noexcept -> UnallocatedCString final;
    auto FindServer() const noexcept -> UnallocatedCString final;
    auto FindUnitDefinition() const noexcept -> UnallocatedCString final;
    auto IssuerUpdate() const noexcept -> UnallocatedCString final;
    auto Messagability() const noexcept -> UnallocatedCString final;
    auto MessageLoaded() const noexcept -> UnallocatedCString final;
    auto NymCreated() const noexcept -> UnallocatedCString final;
    auto NymDownload() const noexcept -> UnallocatedCString final;
    auto PairEvent() const noexcept -> UnallocatedCString final;
    auto PeerReplyUpdate() const noexcept -> UnallocatedCString final;
    auto PeerRequestUpdate() const noexcept -> UnallocatedCString final;
    auto PendingBailment() const noexcept -> UnallocatedCString final;
    auto ProcessPushNotification() const noexcept -> UnallocatedCString final;
    auto PushNotification() const noexcept -> UnallocatedCString final;
    auto SeedUpdated() const noexcept -> UnallocatedCString final;
    auto ServerReplyReceived() const noexcept -> UnallocatedCString final;
    auto ServerRequestSent() const noexcept -> UnallocatedCString final;
    auto ServerUpdate() const noexcept -> UnallocatedCString final;
    auto Shutdown() const noexcept -> UnallocatedCString final;
    auto TaskComplete() const noexcept -> UnallocatedCString final;
    auto ThreadUpdate(const UnallocatedCString& thread) const noexcept
        -> UnallocatedCString final;
    auto UnitUpdate() const noexcept -> UnallocatedCString final;
    auto WidgetUpdate() const noexcept -> UnallocatedCString final;
    auto WorkflowAccountUpdate() const noexcept -> UnallocatedCString final;

    Endpoints(const int instance) noexcept;

    ~Endpoints() final = default;

private:
    static constexpr auto version_1_{1};

    using BlockchainMap = robin_hood::
        unordered_flat_map<opentxs::blockchain::Type, UnallocatedCString>;
    const int instance_;
    const BlockchainMap blockchain_block_updated_;
    const BlockchainMap blockchain_filter_updated_;
    const UnallocatedCString process_push_notification_;
    const UnallocatedCString push_notification_;

    auto build_inproc_path(const UnallocatedCString& path, const int version)
        const noexcept -> UnallocatedCString;
    auto build_inproc_path(
        const UnallocatedCString& path,
        const int version,
        const UnallocatedCString& suffix) const noexcept -> UnallocatedCString;

    Endpoints() = delete;
    Endpoints(const Endpoints&) = delete;
    Endpoints(Endpoints&&) = delete;
    auto operator=(const Endpoints&) -> Endpoints& = delete;
    auto operator=(Endpoints&&) -> Endpoints& = delete;
};
}  // namespace opentxs::api::session::imp
