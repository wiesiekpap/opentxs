// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/ui/BlockchainStatistics.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/util/WorkType.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

namespace network
{
class Blockchain;
}  // namespace network
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

class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using BlockchainStatisticsList = List<
    BlockchainStatisticsExternalInterface,
    BlockchainStatisticsInternalInterface,
    BlockchainStatisticsRowID,
    BlockchainStatisticsRowInterface,
    BlockchainStatisticsRowInternal,
    BlockchainStatisticsRowBlank,
    BlockchainStatisticsSortKey,
    BlockchainStatisticsPrimaryID>;

class BlockchainStatistics final : public BlockchainStatisticsList,
                                   Worker<BlockchainStatistics>
{
public:
    BlockchainStatistics(
        const api::client::Manager& api,
        const SimpleCallback& cb) noexcept;

    ~BlockchainStatistics() final;

private:
    friend Worker<BlockchainStatistics>;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        balance = value(WorkType::BlockchainWalletUpdated),
        blockheader = value(WorkType::BlockchainNewHeader),
        activepeer = value(WorkType::BlockchainPeerAdded),
        reorg = value(WorkType::BlockchainReorg),
        statechange = value(WorkType::BlockchainStateChange),
        filter = value(WorkType::BlockchainNewFilter),
        block = value(WorkType::BlockchainBlockDownloadQueue),
        connectedpeer = value(WorkType::BlockchainPeerConnected),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const api::network::Blockchain& blockchain_;

    auto construct_row(
        const BlockchainStatisticsRowID& id,
        const BlockchainStatisticsSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto custom(const BlockchainStatisticsRowID& chain) const noexcept
        -> CustomData;

    auto pipeline(const Message& in) noexcept -> void;
    auto process_chain(BlockchainStatisticsRowID chain) noexcept -> void;
    auto process_state(const Message& in) noexcept -> void;
    auto process_work(const Message& in) noexcept -> void;
    auto startup() noexcept -> void;

    BlockchainStatistics() = delete;
    BlockchainStatistics(const BlockchainStatistics&) = delete;
    BlockchainStatistics(BlockchainStatistics&&) = delete;
    auto operator=(const BlockchainStatistics&)
        -> BlockchainStatistics& = delete;
    auto operator=(BlockchainStatistics&&) -> BlockchainStatistics& = delete;
};
}  // namespace opentxs::ui::implementation
