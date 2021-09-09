// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Getblocks::"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PGetblocks(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Getblocks*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocks;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    ReturnType::Raw raw_item;

    auto expectedSize = sizeof(raw_item.version_);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocks 1")
            .Flush();

        return nullptr;
    }
    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(
        static_cast<void*>(&raw_item.version_), it, sizeof(raw_item.version_));
    it += sizeof(raw_item.version_);
    bitcoin::ProtocolVersionUnsigned version = raw_item.version_.value();
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocks 1")
            .Flush();

        return nullptr;
    }

    std::vector<OTData> header_hashes;

    std::size_t hashCount{0};
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, hashCount);

    if (!decodedSize) {
        LogOutput(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    if (hashCount > 0) {
        for (std::size_t ii = 0; ii < hashCount; ii++) {
            expectedSize += sizeof(bitcoin::BlockHeaderHashField);

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Header hash entries incomplete at entry index ")(ii)
                    .Flush();

                return nullptr;
            }

            header_hashes.push_back(
                Data::Factory(it, sizeof(bitcoin::BlockHeaderHashField)));
            it += sizeof(bitcoin::BlockHeaderHashField);
        }
    }
    // --------------------------------------------------------
    expectedSize += sizeof(bitcoin::BlockHeaderHashField);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Stop hash entry missing or incomplete")
            .Flush();

        return nullptr;
    }

    auto stop_hash = Data::Factory(it, sizeof(bitcoin::BlockHeaderHashField));
    it += sizeof(bitcoin::BlockHeaderHashField);

    try {
        return new ReturnType(
            api,
            std::move(pHeader),
            version,
            header_hashes,
            std::move(stop_hash));
    } catch (...) {
        LogOutput("opentxs::factory::")(__func__)(": Checksum failure").Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PGetblocks(
    const api::Core& api,
    const blockchain::Type network,
    const std::uint32_t version,
    const std::vector<OTData>& header_hashes,
    const Data& stop_hash) -> blockchain::p2p::bitcoin::message::Getblocks*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocks;

    return new ReturnType(api, network, version, header_hashes, stop_hash);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
// We have all the data members to create the message from scratch (for sending)
Getblocks::Getblocks(
    const api::Core& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersionUnsigned version,
    const std::vector<OTData>& header_hashes,
    const Data& stop_hash) noexcept
    : Message(api, network, bitcoin::Command::getblocks)
    , version_(version)
    , header_hashes_(header_hashes)
    , stop_hash_(Data::Factory(stop_hash))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Getblocks::Getblocks(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersionUnsigned version,
    const std::vector<OTData>& header_hashes,
    const Data& stop_hash) noexcept(false)
    : Message(api, std::move(header))
    , version_(version)
    , header_hashes_(header_hashes)
    , stop_hash_(Data::Factory(stop_hash))
{
    verify_checksum();
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message
