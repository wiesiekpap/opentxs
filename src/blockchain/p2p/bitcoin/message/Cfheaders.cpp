// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Cfheaders.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::bitcoin::message::implemenetation::Cfheaders::"

namespace opentxs::factory
{
auto BitcoinP2PCfheaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Cfheaders*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfheaders;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    ReturnType::BitcoinFormat raw;
    auto expectedSize = sizeof(raw);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(": Payload too short (begin)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&raw), it, sizeof(raw));
    it += sizeof(raw);
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    std::size_t count{0};
    const bool haveCount =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == haveCount) {
        LogOutput(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    std::vector<blockchain::filter::pHash> headers{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += sizeof(bitcoin::message::HashField);

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Filter header entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            headers.emplace_back(
                Data::Factory(it, sizeof(bitcoin::message::HashField)));
            it += sizeof(bitcoin::message::HashField);
        }
    }

    return new ReturnType(
        api,
        std::move(pHeader),
        raw.Type(header.Network()),
        raw.Stop(),
        raw.Previous(),
        headers);
}

auto BitcoinP2PCfheaders(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::filter::Type type,
    const blockchain::block::Hash& stop,
    const ReadView previous,
    const std::vector<blockchain::filter::pHash>& headers)
    -> blockchain::p2p::bitcoin::message::internal::Cfheaders*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Cfheaders;

    return new ReturnType(api, network, type, stop, previous, headers);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Cfheaders::Cfheaders(
    const api::Core& api,
    const blockchain::Type network,
    const filter::Type type,
    const block::Hash& stop,
    const ReadView previousHeader,
    const std::vector<filter::pHash>& headers) noexcept
    : Message(api, network, bitcoin::Command::cfheaders)
    , type_(type)
    , stop_(stop)
    , previous_(api_.Factory().Data(previousHeader))
    , payload_(headers)
{
    init_hash();
}

Cfheaders::Cfheaders(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const filter::Type type,
    const block::Hash& stop,
    const filter::Header& previous,
    const std::vector<filter::pHash>& headers) noexcept
    : Message(api, std::move(header))
    , type_(type)
    , stop_(stop)
    , previous_(previous)
    , payload_(headers)
{
}

auto Cfheaders::payload() const noexcept -> OTData
{
    try {
        BitcoinFormat raw(header().Network(), type_, stop_, previous_);
        auto output = Data::Factory(&raw, sizeof(raw));
        const auto size = CompactSize(payload_.size()).Encode();
        output->Concatenate(size.data(), size.size());

        for (const auto& header : payload_) { output += header; }

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
