// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"

namespace opentxs::blockchain::block
{
struct Block::ParsedPatterns {
    std::vector<Space> data_;
    std::map<ReadView, Patterns::const_iterator> map_;

    ParsedPatterns(const Block::Patterns& in) noexcept;
};

auto SetIntersection(
    const api::Core& api,
    const ReadView txid,
    const Block::ParsedPatterns& patterns,
    const std::vector<Space>& compare) noexcept -> Block::Matches;
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::bitcoin::internal
{
auto Opcode(const OP opcode) noexcept(false) -> ScriptElement;
auto PushData(const ReadView data) noexcept(false) -> ScriptElement;

struct Header : virtual public bitcoin::Header {
};
}  // namespace opentxs::blockchain::block::bitcoin::internal

namespace opentxs::factory
{
auto GenesisBlockHeader(
    const api::Core& api,
    const blockchain::Type type) noexcept
    -> std::unique_ptr<blockchain::block::Header>;
}  // namespace opentxs::factory
