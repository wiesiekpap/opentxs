// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Filterload.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::bitcoin::message::implementation::Filterload::"

namespace opentxs::factory
{
auto BitcoinP2PFilterload(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Filterload*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filterload;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    std::unique_ptr<blockchain::BloomFilter> pFilter{
        factory::BloomFilter(api, Data::Factory(payload, size))};

    if (false == bool(pFilter)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid filter").Flush();

        return nullptr;
    }

    return new ReturnType(api, std::move(pHeader), *pFilter);
}

auto BitcoinP2PFilterload(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::BloomFilter& filter)
    -> blockchain::p2p::bitcoin::message::internal::Filterload*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filterload;

    return new ReturnType(api, network, filter);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Filterload::Filterload(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::BloomFilter& filter) noexcept
    : Message(api, network, bitcoin::Command::filterload)
    , payload_(filter)
{
    init_hash();
}

Filterload::Filterload(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const blockchain::BloomFilter& filter) noexcept
    : Message(api, std::move(header))
    , payload_(filter)
{
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
