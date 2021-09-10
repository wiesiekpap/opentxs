// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"

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
namespace block
{
class OPENTXS_EXPORT Block
{
public:
    using FilterType = filter::Type;
    using Subchain = blockchain::crypto::Subchain;
    using SubchainID = std::pair<Subchain, OTIdentifier>;
    using ElementID = std::pair<Bip32Index, SubchainID>;
    using Pattern = std::pair<ElementID, Space>;
    using Patterns = std::vector<Pattern>;
    using Match = std::pair<pTxid, ElementID>;
    using InputMatch = std::tuple<pTxid, Outpoint, ElementID>;
    using InputMatches = std::vector<InputMatch>;
    using OutputMatches = std::vector<Match>;
    using Matches = std::pair<InputMatches, OutputMatches>;
    using KeyID = blockchain::crypto::Key;
    using ContactID = OTIdentifier;
    using KeyData = std::map<KeyID, std::pair<ContactID, ContactID>>;

    struct ParsedPatterns;

    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(const FilterType style) const noexcept
        -> std::vector<Space> = 0;
    virtual auto FindMatches(
        const FilterType type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches = 0;
    virtual auto Header() const noexcept -> const block::Header& = 0;
    virtual auto ID() const noexcept -> const block::Hash& = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    virtual auto Serialize(AllocateOutput bytes) const noexcept -> bool = 0;

    virtual ~Block() = default;

protected:
    Block() noexcept = default;

private:
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
