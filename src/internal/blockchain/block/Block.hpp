// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
class Header;
struct Outpoint;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::block
{
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

struct ParsedPatterns {
    std::vector<Space> data_;
    std::map<ReadView, Patterns::const_iterator> map_;

    ParsedPatterns(const Patterns& in) noexcept;
};
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::internal
{
struct Block : virtual public block::Block {
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> = 0;
    virtual auto FindMatches(
        const filter::Type type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches = 0;

    ~Block() override = default;
};

auto SetIntersection(
    const api::Core& api,
    const ReadView txid,
    const ParsedPatterns& patterns,
    const std::vector<Space>& compare) noexcept -> Matches;
}  // namespace opentxs::blockchain::block::internal

namespace opentxs::factory
{
auto GenesisBlockHeader(
    const api::Core& api,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::block::Header>;
}  // namespace opentxs::factory
