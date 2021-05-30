// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_HEADERORACLE_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_HEADERORACLE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <set>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
class Header;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace node
{
class HeaderOracle
{
public:
    using Hashes = std::vector<block::pHash>;
    using Positions = std::vector<block::Position>;

    /// Throws std::out_of_range for invalid type
    OPENTXS_EXPORT static const block::Hash& GenesisBlockHash(
        const blockchain::Type type);

    /** Query a partial set of ancestors of a target block
     *
     *  This function always returns at least one position.
     *
     *  The first position returned will be the newest common ancestor between
     *  the target position and the previous chain as indicated by the start
     *  argument. If no reorg has occurred this will be the start argument.
     *
     *  Subsequent positions represent every block following the common ancestor
     *  up to and including the specified stop position, in ascending order.
     *
     *  \throws std::runtime_error if either of the specified positions does not
     *  exist in the database
     */
    OPENTXS_EXPORT virtual Positions Ancestors(
        const block::Position& start,
        const block::Position& target,
        const std::size_t limit = 0) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual block::Position BestChain() const noexcept = 0;
    /** Determine which blocks have updated since the provided position
     *
     *  This function always returns at least one position.
     *
     *  The first position returned will be the newest common ancestor between
     *  the current best chain and the previous chain as indicated by the tip
     *  argument. If no reorg has occurred this will be the tip argument.
     *
     *  Subsequent positions represent every block following the common ancestor
     *  up to and including the current best tip, in ascending order.
     *
     *  \throws std::runtime_error if the specified tip does not exist in the
     *  database
     */
    OPENTXS_EXPORT virtual Positions BestChain(
        const block::Position& tip,
        const std::size_t limit = 0) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual block::pHash BestHash(
        const block::Height height) const noexcept = 0;
    OPENTXS_EXPORT virtual Hashes BestHashes(
        const block::Height start,
        const std::size_t limit = 0) const noexcept = 0;
    OPENTXS_EXPORT virtual Hashes BestHashes(
        const block::Height start,
        const block::Hash& stop,
        const std::size_t limit = 0) const noexcept = 0;
    OPENTXS_EXPORT virtual Hashes BestHashes(
        const Hashes& previous,
        const block::Hash& stop,
        const std::size_t limit) const noexcept = 0;
    /** Determine how which ancestors of a orphaned tip must be rolled back
     *  due to a chain reorg
     *
     *  If the provided tip is in the best chain, the returned vector will be
     *  empty.
     *
     *  Otherwise it will contain a list of orphaned block positions in
     *  descending order starting from the provided tip. The parent block hash
     *  of the block indicated by the final element in the vector
     *  is in the best chain.
     *
     *  \throws std::runtime_error if the provided position is not a descendant
     *  of this chain's genesis block
     */
    OPENTXS_EXPORT virtual Positions CalculateReorg(
        const block::Position tip) const noexcept(false) = 0;
    /** Test block position for membership in the best chain
     *
     *  returns {parent position, best position}
     *
     *  parent position is the input block position if that position is in the
     * best chain, otherwise it is the youngest common ancestor of the input
     * block and best chain
     */
    OPENTXS_EXPORT virtual std::pair<block::Position, block::Position>
    CommonParent(const block::Position& input) const noexcept = 0;
    OPENTXS_EXPORT virtual block::Position GetCheckpoint() const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsInBestChain(
        const block::Hash& hash) const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsInBestChain(
        const block::Position& position) const noexcept = 0;
    OPENTXS_EXPORT virtual std::unique_ptr<block::Header> LoadHeader(
        const block::Hash& hash) const noexcept = 0;
    OPENTXS_EXPORT virtual Hashes RecentHashes() const noexcept = 0;
    OPENTXS_EXPORT virtual std::set<block::pHash> Siblings() const noexcept = 0;

    OPENTXS_EXPORT virtual bool AddCheckpoint(
        const block::Height position,
        const block::Hash& requiredHash) noexcept = 0;
    OPENTXS_EXPORT virtual bool AddHeader(
        std::unique_ptr<block::Header>) noexcept = 0;
    OPENTXS_EXPORT virtual bool AddHeaders(
        std::vector<std::unique_ptr<block::Header>>&) noexcept = 0;
    OPENTXS_EXPORT virtual bool DeleteCheckpoint() noexcept = 0;

    OPENTXS_EXPORT virtual ~HeaderOracle() = default;

protected:
    HeaderOracle() noexcept = default;

private:
    HeaderOracle(const HeaderOracle&) = delete;
    HeaderOracle(HeaderOracle&&) = delete;
    HeaderOracle& operator=(const HeaderOracle&) = delete;
    HeaderOracle& operator=(HeaderOracle&&) = delete;
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
#endif
