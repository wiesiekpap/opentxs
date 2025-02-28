// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/circular_buffer.hpp>
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Position;
}  // namespace block

namespace database
{
class Cfilter;
}  // namespace database

namespace node
{
namespace filteroracle
{
class BlockIndexer;
}  // namespace filteroracle

namespace internal
{
class BlockOracle;
class Manager;
struct Config;
}  // namespace internal

class HeaderOracle;
}  // namespace node

class GCS;
}  // namespace blockchain

namespace network
{
namespace p2p
{
class Data;
}  // namespace p2p

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

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::node::implementation
{
class FilterOracle final : virtual public node::internal::FilterOracle
{
public:
    class FilterDownloader;
    class HeaderDownloader;
    class SyncIndexer;
    struct SyncClientFilterData;

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        block = value(WorkType::BlockchainNewHeader),
        reorg = value(WorkType::BlockchainReorg),
        reset_filter_tip = OT_ZMQ_INTERNAL_SIGNAL + 0,
        heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        full_block = OT_ZMQ_NEW_FULL_BLOCK_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    static auto to_str(Work) -> std::string;

    auto DefaultType() const noexcept -> cfilter::Type final
    {
        return default_type_;
    }
    auto FilterTip(const cfilter::Type type) const noexcept
        -> block::Position final;
    auto GetFilterJob() const noexcept -> CfilterJob final;
    auto GetHeaderJob() const noexcept -> CfheaderJob final;
    auto Heartbeat() const noexcept -> void final;
    auto LoadFilter(
        const cfilter::Type type,
        const block::Hash& block,
        alloc::Default alloc) const noexcept -> GCS final;
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks) const noexcept -> Vector<GCS> final;
    auto LoadFilterHeader(const cfilter::Type type, const block::Hash& block)
        const noexcept -> cfilter::Header final;
    auto LoadFilterOrResetTip(
        const cfilter::Type type,
        const block::Position& position,
        alloc::Default alloc) const noexcept -> GCS final;
    auto ProcessBlock(const bitcoin::block::Block& block) const noexcept
        -> bool final;
    auto ProcessBlock(
        const cfilter::Type type,
        const bitcoin::block::Block& block,
        alloc::Default alloc) const noexcept -> GCS final;
    auto ProcessSyncData(
        const block::Hash& prior,
        const Vector<block::Hash>& hashes,
        const network::p2p::Data& data) const noexcept -> void final;
    auto Tip(const cfilter::Type type) const noexcept -> block::Position final;

    auto Shutdown() noexcept -> void final;
    auto Start() noexcept -> void final;

    FilterOracle(
        const api::Session& api,
        const internal::Config& config,
        const internal::Manager& node,
        const HeaderOracle& header,
        const internal::BlockOracle& block,
        database::Cfilter& database,
        const blockchain::Type chain,
        const blockchain::cfilter::Type filter,
        const UnallocatedCString& shutdown) noexcept;
    FilterOracle() = delete;
    FilterOracle(const FilterOracle&) = delete;
    FilterOracle(FilterOracle&&) = delete;
    auto operator=(const FilterOracle&) -> FilterOracle& = delete;
    auto operator=(FilterOracle&&) -> FilterOracle& = delete;

    ~FilterOracle() final;

private:
    friend internal::FilterOracle;

    using FilterHeaderHex = UnallocatedCString;
    using FilterHeaderMap = UnallocatedMap<cfilter::Type, FilterHeaderHex>;
    using ChainMap = UnallocatedMap<block::Height, FilterHeaderMap>;
    using CheckpointMap = UnallocatedMap<blockchain::Type, ChainMap>;
    using OutstandingMap = UnallocatedMap<int, std::atomic_int>;

    static const CheckpointMap filter_checkpoints_;

    const api::Session& api_;
    const internal::Manager& node_;
    const HeaderOracle& header_;
    database::Cfilter& database_;
    const network::zeromq::socket::Publish& filter_notifier_;
    const blockchain::Type chain_;
    const cfilter::Type default_type_;
    mutable std::recursive_mutex lock_;
    OTZMQPublishSocket new_filters_;
    const filteroracle::NotifyCallback cb_;
    mutable std::unique_ptr<FilterDownloader> filter_downloader_;
    mutable std::unique_ptr<HeaderDownloader> header_downloader_;
    mutable std::unique_ptr<filteroracle::BlockIndexer> block_indexer_;
    mutable Time last_sync_progress_;
    mutable UnallocatedMap<cfilter::Type, block::Position> last_broadcast_;
    mutable JobCounter outstanding_jobs_;
    std::atomic_bool running_;

    auto new_tip(
        const rLock&,
        const cfilter::Type type,
        const block::Position& tip) const noexcept -> void;
    auto reset_tips_to(
        const cfilter::Type type,
        const block::Position& position,
        const std::optional<bool> resetHeader = std::nullopt,
        const std::optional<bool> resetFilter = std::nullopt) const noexcept
        -> bool;
    auto reset_tips_to(
        const cfilter::Type type,
        const block::Position& headerTip,
        const block::Position& position,
        const std::optional<bool> resetHeader = std::nullopt) const noexcept
        -> bool;
    auto reset_tips_to(
        const cfilter::Type type,
        const block::Position& headerTip,
        const block::Position& filterTip,
        const block::Position& position,
        std::optional<bool> resetHeader = std::nullopt,
        std::optional<bool> resetFilter = std::nullopt) const noexcept -> bool;

    auto compare_header_to_checkpoint(
        const block::Position& block,
        const cfilter::Header& header) noexcept -> block::Position;
    auto compare_tips_to_checkpoint() noexcept -> void;
    auto compare_tips_to_header_chain() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::implementation
