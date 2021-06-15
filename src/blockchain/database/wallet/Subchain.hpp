// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <mutex>
#include <optional>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"

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
    using Parent = node::internal::WalletDatabase;
    using SubchainID = Identifier;
    using pSubchainID = OTIdentifier;
    using SubchainIndex = Parent::SubchainIndex;
    using pSubchainIndex = Parent::pSubchainIndex;
    using NodeID = Parent::NodeID;
    using pNodeID = Parent::pNodeID;
    using Subchain = Parent::Subchain;
    using FilterType = Parent::FilterType;
    using Patterns = Parent::Patterns;
    using ElementMap = Parent::ElementMap;
    using MatchingIndices = Parent::MatchingIndices;

    auto GetIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> pSubchainIndex;
    auto GetSubchainID(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> pSubchainID;
    auto GetMutex() const noexcept -> std::mutex&;
    auto GetPatterns(const SubchainIndex& subchain) const noexcept -> Patterns;
    auto GetUntestedPatterns(
        const SubchainIndex& subchain,
        const ReadView blockID) const noexcept -> Patterns;
    auto Reorg(
        const Lock& lock,
        const SubchainIndex& subchain,
        const block::Height lastGoodHeight) const noexcept(false) -> bool;
    auto SetDefaultFilterType(const FilterType type) const noexcept -> bool;
    auto SubchainAddElements(
        const SubchainIndex& subchain,
        const ElementMap& elements) const noexcept -> bool;
    auto SubchainLastIndexed(const SubchainIndex& subchain) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastScanned(const SubchainIndex& subchain) const noexcept
        -> block::Position;
    auto SubchainMatchBlock(
        const SubchainIndex& subchain,
        const MatchingIndices& indices,
        const ReadView blockID) const noexcept -> bool;
    auto SubchainSetLastScanned(
        const SubchainIndex& subchain,
        const block::Position& position) const noexcept -> bool;
    auto Type() const noexcept -> FilterType;

    SubchainData(const api::Core& api) noexcept;

    ~SubchainData();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
