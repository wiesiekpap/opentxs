// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Headers.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: keep
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Headers::"

namespace opentxs::factory
{
auto BitcoinP2PHeaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Headers*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Headers;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto& header = *pHeader;
    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Payload too short (compactsize)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    auto count = std::size_t{0};
    const bool decodedSize =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == decodedSize) {
        LogOutput(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    std::vector<std::unique_ptr<blockchain::block::bitcoin::Header>> headers{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += 81;

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Block Header entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            auto pHeader = factory::BitcoinBlockHeader(
                api,
                header.Network(),
                ReadView{reinterpret_cast<const char*>(it), 80});

            if (pHeader) {
                headers.emplace_back(std::move(pHeader));

                it += 81;
            } else {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Invalid header received at index ")(i)
                    .Flush();

                break;
            }
        }
    }

    return new ReturnType(api, std::move(pHeader), std::move(headers));
}

auto BitcoinP2PHeaders(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<std::unique_ptr<blockchain::block::bitcoin::Header>>&& headers)
    -> blockchain::p2p::bitcoin::message::internal::Headers*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Headers;

    return new ReturnType(api, network, std::move(headers));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Headers::Headers(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<std::unique_ptr<value_type>>&& headers) noexcept
    : Message(api, network, bitcoin::Command::headers)
    , payload_(std::move(headers))
{
    init_hash();
}

Headers::Headers(
    const api::Core& api,
    std::unique_ptr<Header> header,
    std::vector<std::unique_ptr<value_type>>&& headers) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(headers))
{
}

auto Headers::payload() const noexcept -> OTData
{
    const auto null = Data::Factory("0x00", Data::Mode::Hex);
    auto output = Data::Factory(CompactSize(payload_.size()).Encode());

    for (const auto& pHeader : payload_) {
        OT_ASSERT(pHeader);

        const auto& header = *pHeader;
        output += header.Encode();
        output += null;
    }

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
