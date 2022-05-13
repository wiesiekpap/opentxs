// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/header/Header.hpp"

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::Session& api,
    const opentxs::blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    opentxs::blockchain::block::Hash&& merkle,
    const AbortFunction abort) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}

auto BitcoinBlockHeader(
    const api::Session& api,
    const proto::BlockchainBlockHeader& serialized) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}

auto BitcoinBlockHeader(
    const api::Session& api,
    const blockchain::Type chain,
    const ReadView raw) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}

auto BitcoinBlockHeader(
    const api::Session& api,
    const blockchain::Type chain,
    const blockchain::block::Hash& merkle,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}
}  // namespace opentxs::factory
