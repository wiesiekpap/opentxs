// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/interface/ui/BlockchainStatistics.hpp"
#include "opentxs/interface/ui/Blockchains.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
class Blockchain;
}  // namespace network

namespace session
{
class Client;
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
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
                                   public Worker<api::session::Client>
{
public:
    BlockchainStatistics(
        const api::session::Client& api,
        const SimpleCallback& cb) noexcept;

    ~BlockchainStatistics() final;

protected:
    auto pipeline(Message&& in) noexcept -> void final;
    auto state_machine() noexcept -> bool final;

private:
    auto shut_down() noexcept -> void;

private:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        blockheader = value(WorkType::BlockchainNewHeader),
        activepeer = value(WorkType::BlockchainPeerAdded),
        reorg = value(WorkType::BlockchainReorg),
        statechange = value(WorkType::BlockchainStateChange),
        filter = value(WorkType::BlockchainNewFilter),
        block = value(WorkType::BlockchainBlockDownloadQueue),
        connectedpeer = value(WorkType::BlockchainPeerConnected),
        balance = value(WorkType::BlockchainWalletUpdated),
        timer = OT_ZMQ_INTERNAL_SIGNAL + 0,
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };
    using CachedData = std::tuple<
        blockchain::block::Height,
        blockchain::block::Height,
        std::size_t,
        std::size_t,
        std::size_t,
        blockchain::Amount>;

    const api::network::Blockchain& blockchain_;
    Map<BlockchainStatisticsRowID, CachedData> cache_;
    Timer timer_;

    auto construct_row(
        const BlockchainStatisticsRowID& id,
        const BlockchainStatisticsSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto custom(const BlockchainStatisticsRowID& chain) noexcept -> CustomData;
    auto get_cache(const BlockchainStatisticsRowID& chain) noexcept(false)
        -> CachedData&;
    auto process_balance(const Message& in) noexcept -> void;
    auto process_block(const Message& in) noexcept -> void;
    auto process_block_header(const Message& in) noexcept -> void;
    auto process_cfilter(const Message& in) noexcept -> void;
    auto process_chain(BlockchainStatisticsRowID chain) noexcept -> void;
    auto process_reorg(const Message& in) noexcept -> void;
    auto process_state(const Message& in) noexcept -> void;
    auto process_timer(const Message& in) noexcept -> void;
    auto process_work(const Message& in) noexcept -> void;
    auto reset_timer() noexcept -> void;
    auto startup() noexcept -> void;

    BlockchainStatistics() = delete;
    BlockchainStatistics(const BlockchainStatistics&) = delete;
    BlockchainStatistics(BlockchainStatistics&&) = delete;
    auto operator=(const BlockchainStatistics&)
        -> BlockchainStatistics& = delete;
    auto operator=(BlockchainStatistics&&) -> BlockchainStatistics& = delete;
};
}  // namespace opentxs::ui::implementation
