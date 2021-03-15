// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
class SubchainData
{
public:
    using Common = api::client::blockchain::database::implementation::Database;
    using Parent = client::internal::WalletDatabase;
    using SubchainID = Identifier;
    using pSubchainID = OTIdentifier;
    using NodeID = Parent::NodeID;
    using Subchain = Parent::Subchain;
    using FilterType = Parent::FilterType;
    using Patterns = Parent::Patterns;
    using ElementMap = Parent::ElementMap;
    using MatchingIndices = Parent::MatchingIndices;

    auto GetID(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> pSubchainID;
    auto GetID(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainID;
    auto GetID(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> pSubchainID;
    auto GetMutex() const noexcept -> std::mutex&;
    auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> Patterns;
    auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> Patterns;
    auto Reorg(
        const Lock& lock,
        const SubchainID& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool;
    auto SetDefaultFilterType(const FilterType type) const noexcept -> bool;
    auto SubchainAddElements(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber;
    auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position;
    auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position;
    auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version) const noexcept -> bool;
    auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool;
    auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool;
    auto Type() const noexcept -> FilterType;

    SubchainData(const api::Core& api) noexcept;

    ~SubchainData();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
