// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Inv.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>
#include <vector>

#include "blockchain/bitcoin/Inventory.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

//#define OT_METHOD "
// opentxs::blockchain::p2p::bitcoin::message::implementation::Inv::"

namespace opentxs::factory
{
auto BitcoinP2PInv(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::internal::Inv*
{
    namespace bitcoin = blockchain::p2p::bitcoin::message;
    using ReturnType = bitcoin::implementation::Inv;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Inv 1")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::size_t count{0};
    const bool haveCount =
        network::blockchain::bitcoin::DecodeSize(it, expectedSize, size, count);

    if (false == haveCount) {
        LogOutput(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    std::vector<blockchain::bitcoin::Inventory> items{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize += ReturnType::value_type::EncodedSize;

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Inventory entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            items.emplace_back(it, ReturnType::value_type::EncodedSize);
            it += ReturnType::value_type::EncodedSize;
        }
    }

    return new ReturnType(api, std::move(pHeader), std::move(items));
}

auto BitcoinP2PInv(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<blockchain::bitcoin::Inventory>&& payload)
    -> blockchain::p2p::bitcoin::message::internal::Inv*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Inv;

    return new ReturnType(api, network, std::move(payload));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Inv::Inv(
    const api::Core& api,
    const blockchain::Type network,
    std::vector<value_type>&& payload) noexcept
    : Message(api, network, bitcoin::Command::inv)
    , payload_(std::move(payload))
{
    init_hash();
}

Inv::Inv(
    const api::Core& api,
    std::unique_ptr<Header> header,
    std::vector<value_type>&& payload) noexcept
    : Message(api, std::move(header))
    , payload_(std::move(payload))
{
}

auto Inv::payload() const noexcept -> OTData
{
    try {
        auto output = Data::Factory(CompactSize(payload_.size()).Encode());

        for (const auto& item : payload_) { output += item.Encode(); }

        return output;
    } catch (...) {

        return Data::Factory();
    }
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message::implementation
