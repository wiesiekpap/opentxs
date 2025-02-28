// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <tuple>
#include <utility>

#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

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
class Header;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Header;
class Position;
}  // namespace block

namespace database
{
class Header;
}  // namespace database

namespace node
{
class UpdateTransaction;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace p2p
{
class Data;
}  // namespace p2p

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class HeaderOracle final : virtual public node::internal::HeaderOracle
{
public:
    auto Ancestors(
        const block::Position& start,
        const block::Position& target,
        const std::size_t limit) const noexcept(false) -> Positions final;
    auto BestChain() const noexcept -> block::Position final;
    auto BestChain(const block::Position& tip, const std::size_t limit) const
        noexcept(false) -> Positions final;
    auto BestHash(const block::Height height) const noexcept
        -> block::Hash final;
    auto BestHash(const block::Height height, const block::Position& check)
        const noexcept -> block::Hash final;
    auto BestHashes(
        const block::Height start,
        const std::size_t limit,
        alloc::Resource* alloc) const noexcept -> Hashes final;
    auto BestHashes(
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Resource* alloc) const noexcept -> Hashes final;
    auto BestHashes(
        const Hashes& previous,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Resource* alloc) const noexcept -> Hashes final;
    auto CalculateReorg(const block::Position& tip) const noexcept(false)
        -> Positions final;
    auto CalculateReorg(const Lock& lock, const block::Position& tip) const
        noexcept(false) -> Positions final
    {
        return calculate_reorg(lock, tip);
    }
    auto CommonParent(const block::Position& position) const noexcept
        -> std::pair<block::Position, block::Position> final;
    auto GetCheckpoint() const noexcept -> block::Position final;
    auto GetDefaultCheckpoint() const noexcept -> CheckpointData final;
    auto GetMutex() const noexcept -> std::mutex& final { return lock_; }
    auto GetPosition(const block::Height height) const noexcept
        -> block::Position final;
    auto GetPosition(const Lock& lock, const block::Height height)
        const noexcept -> block::Position final
    {
        return get_position(lock, height);
    }
    auto Internal() const noexcept -> const internal::HeaderOracle& final
    {
        return *this;
    }
    auto IsInBestChain(const block::Hash& hash) const noexcept -> bool final;
    auto IsInBestChain(const block::Position& position) const noexcept
        -> bool final;
    auto LoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<bitcoin::block::Header> final;
    auto LoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> final;
    auto RecentHashes(alloc::Resource* alloc) const noexcept -> Hashes final;
    auto Siblings() const noexcept -> UnallocatedSet<block::Hash> final;

    auto AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept -> bool final;
    auto AddHeader(std::unique_ptr<block::Header> header) noexcept
        -> bool final;
    auto AddHeaders(UnallocatedVector<std::unique_ptr<block::Header>>&) noexcept
        -> bool final;
    auto DeleteCheckpoint() noexcept -> bool final;
    auto Init() noexcept -> void final;
    auto Internal() noexcept -> internal::HeaderOracle& final { return *this; }
    auto ProcessSyncData(
        block::Hash& prior,
        Vector<block::Hash>& hashes,
        const network::p2p::Data& data) noexcept -> std::size_t final;

    HeaderOracle(
        const api::Session& api,
        database::Header& database,
        const blockchain::Type type) noexcept;
    HeaderOracle() = delete;
    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) = delete;
    auto operator=(const HeaderOracle&) -> HeaderOracle& = delete;
    auto operator=(HeaderOracle&&) -> HeaderOracle& = delete;

    ~HeaderOracle() final = default;

private:
    struct Candidate {
        bool blacklisted_{false};
        UnallocatedDeque<block::Position> chain_{};
    };

    using Candidates = UnallocatedVector<Candidate>;

    const api::Session& api_;
    database::Header& database_;
    const blockchain::Type chain_;
    mutable std::mutex lock_;

    static auto evaluate_candidate(
        const block::Header& current,
        const block::Header& candidate) noexcept -> bool;

    auto best_chain(const Lock& lock) const noexcept -> block::Position;
    auto best_chain(
        const Lock& lock,
        const block::Position& tip,
        const std::size_t limit) const noexcept -> Positions;
    auto best_hash(const Lock& lock, const block::Height height) const noexcept
        -> block::Hash;
    auto best_hashes(
        const Lock& lock,
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit,
        alloc::Resource* alloc) const noexcept -> Hashes;
    static auto blank_hash() noexcept -> const block::Hash&;
    auto blank_position() const noexcept -> const block::Position&;
    auto calculate_reorg(const Lock& lock, const block::Position& tip) const
        noexcept(false) -> Positions;
    auto common_parent(const Lock& lock, const block::Position& position)
        const noexcept -> std::pair<block::Position, block::Position>;
    auto get_position(const Lock& lock, const block::Height height)
        const noexcept -> block::Position;
    auto is_in_best_chain(const Lock& lock, const block::Hash& hash)
        const noexcept -> std::pair<bool, block::Height>;
    auto is_in_best_chain(const Lock& lock, const block::Position& position)
        const noexcept -> bool;
    auto is_in_best_chain(
        const Lock& lock,
        const block::Height height,
        const block::Hash& hash) const noexcept -> bool;

    auto add_header(
        const Lock& lock,
        UpdateTransaction& update,
        std::unique_ptr<block::Header> header) noexcept -> bool;
    auto apply_checkpoint(
        const Lock& lock,
        const block::Height height,
        UpdateTransaction& update) noexcept -> bool;
    auto choose_candidate(
        const block::Header& current,
        const Candidates& candidates,
        UpdateTransaction& update) noexcept(false) -> std::pair<bool, bool>;
    auto connect_children(
        const Lock& lock,
        block::Header& parentHeader,
        Candidates& candidates,
        Candidate& candidate,
        UpdateTransaction& update) -> void;
    // Returns true if the child is checkpoint blacklisted
    static auto connect_to_parent(
        const Lock& lock,
        const UpdateTransaction& update,
        const block::Header& parent,
        block::Header& child) noexcept -> bool;
    static auto initialize_candidate(
        const Lock& lock,
        const block::Header& best,
        const block::Header& parent,
        UpdateTransaction& update,
        Candidates& candidates,
        block::Header& child,
        const block::Hash& stopHash = {}) noexcept(false) -> Candidate&;
    static auto is_disconnected(
        const block::Hash& parent,
        UpdateTransaction& update) noexcept -> const block::Header*;
    static auto stage_candidate(
        const Lock& lock,
        const block::Header& best,
        Candidates& candidates,
        UpdateTransaction& update,
        block::Header& child) noexcept(false) -> void;
};
}  // namespace opentxs::blockchain::node::implementation
