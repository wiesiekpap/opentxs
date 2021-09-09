// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getcfilters.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::bitcoin::message::implementation::Getcfilters::"
namespace opentxs::factory
{
auto BitcoinP2PGetcfilters(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getcfilters*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getcfilters;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    ReturnType::BitcoinFormat raw;
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(": Payload too short")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    it += sizeof(raw);

    return new ReturnType(
        api,
        std::move(pHeader),
        raw.Type(header.Network()),
        raw.Start(),
        raw.Stop());
}

auto BitcoinP2PGetcfilters(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Height start,
    const blockchain::block::Hash& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getcfilters*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getcfilters;

    return new ReturnType(api, network, type, start, stop);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getcfilters::Getcfilters(
    const api::Core& api,
    const blockchain::Type network,
    const filter::Type type,
    const block::Height start,
    const filter::Hash& stop) noexcept
    : Message(api, network, bitcoin::Command::getcfilters)
    , type_(type)
    , start_(start)
    , stop_(stop)
{
    init_hash();
}

Getcfilters::Getcfilters(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const filter::Type type,
    const block::Height start,
    const filter::Hash& stop) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , start_(start)
    , stop_(stop)
{
}

auto Getcfilters::payload() const noexcept -> OTData
{
    try {
        BitcoinFormat raw(header().Network(), type_, start_, stop_);
        auto output = Data::Factory(&raw, sizeof(raw));

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
