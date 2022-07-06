// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <chrono>
#include <cstddef>
#include <exception>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/blockoracle/Cache.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/block/Validator.hpp"
#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/node/BlockBatch.hpp"
#include "internal/blockchain/node/BlockOracle.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Actor.hpp"
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

namespace node
{
namespace blockoracle
{
class BlockBatch;
class BlockDownloader;
}  // namespace blockoracle

namespace internal
{
class BlockBatch;
class Manager;
}  // namespace internal

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
class BlockOracle::Imp final : public Actor<BlockOracleJobs>
{
public:
    auto DownloadQueue() const noexcept -> std::size_t
    {
        return cache_.lock_shared()->DownloadQueue();
    }
    auto Endpoint() const noexcept -> std::string_view
    {
        return submit_endpoint_;
    }

    // TODO assess the thread safety
    auto GetBlockBatch(boost::shared_ptr<Imp> me) const noexcept -> BlockBatch;
    // TODO assess the thread safety
    auto GetBlockJob() const noexcept -> BlockJob;

    auto Heartbeat() const noexcept -> void;

    // TODO assess the thread safety
    auto LoadBitcoin(const block::Hash& block) const noexcept
        -> BitcoinBlockResult;
    // TODO assess the thread safety
    auto LoadBitcoin(const Vector<block::Hash>& hashes) const noexcept
        -> BitcoinBlockResults;

    auto SubmitBlock(const ReadView in) const noexcept -> void;
    auto Tip() const noexcept -> block::Position { return db_.BlockTip(); }

    // TODO check why this is always true
    auto Validate(const bitcoin::block::Block& block) const noexcept -> bool
    {
        return validator_->Validate(block);
    }

    auto Init(boost::shared_ptr<Imp> me) noexcept -> void
    {
        signal_startup(me);
    }
    auto Shutdown() noexcept -> void { return signal_shutdown(); }
    auto StartDownloader() noexcept -> void;

    Imp(const api::Session& api,
        const internal::Manager& node,
        const node::HeaderOracle& header,
        database::Block& db,
        const blockchain::Type chain,
        const std::string_view parent,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;

    ~Imp() final;

protected:
    auto do_startup() noexcept -> void override;
    auto do_shutdown() noexcept -> void override;
    auto pipeline(const Work work, Message&& msg) noexcept -> void override;
    auto work() noexcept -> bool override;
    auto to_str(Work w) const noexcept -> std::string final;

private:
    using Task = BlockOracleJobs;
    using Cache =
        libguarded::shared_guarded<blockoracle::Cache, std::shared_mutex>;

    const api::Session& api_;
    const internal::Manager& node_;
    const database::Block& db_;
    const CString submit_endpoint_;
    const std::unique_ptr<const block::Validator> validator_;
    const std::unique_ptr<blockoracle::BlockDownloader> block_downloader_;
    mutable Cache cache_;

    static auto get_validator(
        const blockchain::Type chain,
        const node::HeaderOracle& headers) noexcept
        -> std::unique_ptr<const block::Validator>;

    Imp(const api::Session& api,
        const internal::Manager& node,
        const node::HeaderOracle& header,
        database::Block& db,
        const blockchain::Type chain,
        const std::string_view parent,
        const network::zeromq::BatchID batch,
        CString&& submitEndpoint,
        allocator_type alloc) noexcept;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::blockchain::node::internal
