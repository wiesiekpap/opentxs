// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/session/Endpoints.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>

#include "internal/api/session/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"

namespace opentxs::factory
{
auto EndpointsAPI(const int instance) noexcept
    -> std::unique_ptr<api::session::Endpoints>
{
    using ReturnType = api::session::imp::Endpoints;

    return std::make_unique<ReturnType>(instance);
}
}  // namespace opentxs::factory

namespace opentxs::api::session::imp
{
Endpoints::Endpoints(const int instance) noexcept
    : instance_(instance)
    , blockchain_block_updated_([] {
        auto out = BlockchainMap{};

        for (const auto& chain : opentxs::blockchain::DefinedChains()) {
            out.emplace(chain, opentxs::network::zeromq::MakeArbitraryInproc());
        }

        return out;
    }())
    , blockchain_filter_updated_([] {
        auto out = BlockchainMap{};

        for (const auto& chain : opentxs::blockchain::DefinedChains()) {
            out.emplace(chain, opentxs::network::zeromq::MakeArbitraryInproc());
        }

        return out;
    }())
    , process_push_notification_(
          opentxs::network::zeromq::MakeArbitraryInproc())
    , push_notification_(opentxs::network::zeromq::MakeArbitraryInproc())
{
}

auto Endpoints::build_inproc_path(const std::string& path, const int version)
    const noexcept -> std::string
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
        path, instance_, version);
}

auto Endpoints::build_inproc_path(
    const std::string& path,
    const int version,
    const std::string& suffix) const noexcept -> std::string
{
    return opentxs::network::zeromq::MakeDeterministicInproc(
        path, instance_, version, suffix);
}

auto Endpoints::AccountUpdate() const noexcept -> std::string
{
    static constexpr auto path{"accountupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainAccountCreated() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/account/new"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainBalance() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/balance/interactive"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainBlockAvailable() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/block/available"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainBlockDownloadQueue() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/block/queue"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainBlockUpdated(
    const opentxs::blockchain::Type chain) const noexcept -> std::string
{
    return blockchain_block_updated_.at(chain);
}

auto Endpoints::BlockchainFilterUpdated(
    const opentxs::blockchain::Type chain) const noexcept -> std::string
{
    return blockchain_filter_updated_.at(chain);
}

auto Endpoints::BlockchainMempool() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/mempool"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainNewFilter() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/filter"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainPeer() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/peer/active"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainPeerConnection() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/peer/connected"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainReorg() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/reorg"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainScanProgress() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/scan"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainStateChange() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/state"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainSyncProgress() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/sync"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainSyncServerUpdated() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/sync/db"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainTransactions() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/transactions"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainTransactions(
    const identifier::Nym& nym) const noexcept -> std::string
{
    static constexpr auto prefix{"blockchain/transactions"};
    const auto path = std::string{prefix} + "by_nym/" + nym.str();

    return build_inproc_path(path, version_1_);
}

auto Endpoints::BlockchainWalletUpdated() const noexcept -> std::string
{
    static constexpr auto path{"blockchain/balance"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::ConnectionStatus() const noexcept -> std::string
{
    static constexpr auto path{"connectionstatus"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::ContactUpdate() const noexcept -> std::string
{
    static constexpr auto path{"contactupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::DhtRequestNym() const noexcept -> std::string
{
    static constexpr auto path{"dht/requestnym"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::DhtRequestServer() const noexcept -> std::string
{
    static constexpr auto path{"dht/requestserver"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::DhtRequestUnit() const noexcept -> std::string
{
    static constexpr auto path{"dht/requestunit"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::FindNym() const noexcept -> std::string
{
    static constexpr auto path{"otx/search/nym"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::FindServer() const noexcept -> std::string
{
    static constexpr auto path{"otx/search/server"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::FindUnitDefinition() const noexcept -> std::string
{
    static constexpr auto path{"otx/search/unit"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::IssuerUpdate() const noexcept -> std::string
{
    static constexpr auto path{"issuerupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::Messagability() const noexcept -> std::string
{
    static constexpr auto path{"otx/messagability"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::MessageLoaded() const noexcept -> std::string
{
    static constexpr auto path{"otx/message_loaded"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::NymCreated() const noexcept -> std::string
{
    static constexpr auto path{"nymcreated"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::NymDownload() const noexcept -> std::string
{
    static constexpr auto path{"nymupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::PairEvent() const noexcept -> std::string
{
    static constexpr auto path{"pairevent"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::PeerReplyUpdate() const noexcept -> std::string
{
    static constexpr auto path{"peerreplyupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::PeerRequestUpdate() const noexcept -> std::string
{
    static constexpr auto path{"peerrequestupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::PendingBailment() const noexcept -> std::string
{
    static constexpr auto path{"peerrequest/pendingbailment"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::ProcessPushNotification() const noexcept -> std::string
{
    return process_push_notification_;
}

auto Endpoints::PushNotification() const noexcept -> std::string
{
    return push_notification_;
}

auto Endpoints::ServerReplyReceived() const noexcept -> std::string
{
    static constexpr auto path{"reply/received"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::ServerRequestSent() const noexcept -> std::string
{
    static constexpr auto path{"request/sent"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::ServerUpdate() const noexcept -> std::string
{
    static constexpr auto path{"serverupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::Shutdown() const noexcept -> std::string
{
    static constexpr auto path{"shutdown"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::TaskComplete() const noexcept -> std::string
{
    static constexpr auto path{"taskcomplete"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::ThreadUpdate(const std::string& thread) const noexcept
    -> std::string
{
    static constexpr auto path{"threadupdate"};

    return build_inproc_path(path, version_1_, thread);
}

auto Endpoints::UnitUpdate() const noexcept -> std::string
{
    static constexpr auto path{"unitupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::WidgetUpdate() const noexcept -> std::string
{
    static constexpr auto path{"ui/widgetupdate"};

    return build_inproc_path(path, version_1_);
}

auto Endpoints::WorkflowAccountUpdate() const noexcept -> std::string
{
    static constexpr auto path{"ui/workflowupdate/account"};

    return build_inproc_path(path, version_1_);
}
}  // namespace opentxs::api::session::imp
