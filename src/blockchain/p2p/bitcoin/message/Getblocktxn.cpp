// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getblocktxn.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Getblocktxn::"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PGetblocktxn(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Getblocktxn*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocktxn;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(bitcoin::BlockHeaderHashField);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocktxn 1")
            .Flush();

        return nullptr;
    }
    auto* it{static_cast<const std::byte*>(payload)};
    // --------------------------------------------------------
    OTData block_hash =
        Data::Factory(it, sizeof(bitcoin::BlockHeaderHashField));
    it += sizeof(bitcoin::BlockHeaderHashField);
    // --------------------------------------------------------
    // Next load up the transaction count (CompactSize)

    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Getblocktxn 1")
            .Flush();

        return nullptr;
    }

    std::size_t indicesCount{0};
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, indicesCount);

    if (!decodedSize) {
        LogOutput(__func__)(": CompactSize incomplete").Flush();

        return nullptr;
    }

    std::vector<std::size_t> txn_indices;

    if (indicesCount > 0) {
        for (std::size_t ii = 0; ii < indicesCount; ii++) {
            expectedSize += sizeof(std::byte);

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Txn index entries incomplete at entry index ")(ii)
                    .Flush();

                return nullptr;
            }

            std::size_t txnIndex{0};
            const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
                it, expectedSize, size, txnIndex);

            if (!decodedSize) {
                LogOutput(__func__)(": CompactSize incomplete").Flush();

                return nullptr;
            }

            txn_indices.push_back(txnIndex);
        }
    }
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), block_hash, txn_indices);
    } catch (...) {
        LogOutput("opentxs::factory::")(__func__)(": Checksum failure").Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PGetblocktxn(
    const api::Core& api,
    const blockchain::Type network,
    const Data& block_hash,
    const std::vector<std::size_t>& txn_indices)
    -> blockchain::p2p::bitcoin::message::Getblocktxn*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Getblocktxn;

    return new ReturnType(api, network, block_hash, txn_indices);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{

auto Getblocktxn::payload() const noexcept -> OTData
{
    try {
        auto output = Data::Factory(block_hash_);
        // ---------------------------
        const auto size = CompactSize(txn_indices_.size()).Encode();
        output->Concatenate(size.data(), size.size());
        // ---------------------------
        for (const auto& index : txn_indices_) {
            const auto size = CompactSize(index).Encode();
            output->Concatenate(size.data(), size.size());
        }

        return output;
    } catch (...) {
        return Data::Factory();
    }
}

// We have all the data members to create the message from scratch (for sending)
Getblocktxn::Getblocktxn(
    const api::Core& api,
    const blockchain::Type network,
    const Data& block_hash,
    const std::vector<std::size_t>& txn_indices) noexcept
    : Message(api, network, bitcoin::Command::getblocktxn)
    , block_hash_(Data::Factory(block_hash))
    , txn_indices_(txn_indices)
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Getblocktxn::Getblocktxn(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const Data& block_hash,
    const std::vector<std::size_t>& txn_indices) noexcept(false)
    : Message(api, std::move(header))
    , block_hash_(Data::Factory(block_hash))
    , txn_indices_(txn_indices)
{
    verify_checksum();
}

}  // namespace  opentxs::blockchain::p2p::bitcoin::message
