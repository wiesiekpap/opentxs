// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Block.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

//#define OT_METHOD "
// opentxs::blockchain::p2p::bitcoin::message::implementation::Block::"

namespace opentxs::factory
{
auto BitcoinP2PBlock(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Block*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Block;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto raw_block(Data::Factory(payload, size));

    return new ReturnType(api, std::move(pHeader), raw_block);
}

auto BitcoinP2PBlock(
    const api::Core& api,
    const blockchain::Type network,
    const Data& raw_block)
    -> blockchain::p2p::bitcoin::message::internal::Block*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Block;

    return new ReturnType(api, network, raw_block);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Block::Block(
    const api::Core& api,
    const blockchain::Type network,
    const Data& block) noexcept
    : Message(api, network, bitcoin::Command::block)
    , payload_(block)
{
    init_hash();
}

Block::Block(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const Data& block) noexcept
    : Message(api, std::move(header))
    , payload_(block)
{
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
