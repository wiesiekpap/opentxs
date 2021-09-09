// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Getheaders.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Getheaders::"

namespace opentxs::factory
{
auto BitcoinP2PGetheaders(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getheaders;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    blockchain::p2p::bitcoin::ProtocolVersionField version{};
    auto expectedSize = sizeof(version);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Payload too short (version)")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    std::memcpy(reinterpret_cast<std::byte*>(&version), it, sizeof(version));
    it += sizeof(version);
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
        LogOutput(__func__)(": Invalid CompactSize").Flush();

        return nullptr;
    }

    std::vector<blockchain::block::pHash> hashes{};

    if (count > 0) {
        for (std::size_t i{0}; i < count; ++i) {
            expectedSize +=
                sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField);

            if (expectedSize > size) {
                LogOutput("opentxs::factory::")(__func__)(
                    ": Header hash entries incomplete at entry index ")(i)
                    .Flush();

                return nullptr;
            }

            hashes.emplace_back(Data::Factory(
                it, sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField)));
            it += sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField);
        }
    }

    expectedSize += sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Stop hash entry missing or incomplete")
            .Flush();

        return nullptr;
    }

    auto stop = Data::Factory(
        it, sizeof(blockchain::p2p::bitcoin::BlockHeaderHashField));

    return new ReturnType(
        api,
        std::move(pHeader),
        version.value(),
        std::move(hashes),
        std::move(stop));
}

auto BitcoinP2PGetheaders(
    const api::Core& api,
    const blockchain::Type network,
    const blockchain::p2p::bitcoin::ProtocolVersionUnsigned version,
    std::vector<blockchain::block::pHash>&& history,
    blockchain::block::pHash&& stop)
    -> blockchain::p2p::bitcoin::message::internal::Getheaders*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Getheaders;

    return new ReturnType(
        api, network, version, std::move(history), std::move(stop));
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Getheaders::Getheaders(
    const api::Core& api,
    const blockchain::Type network,
    const bitcoin::ProtocolVersionUnsigned version,
    std::vector<block::pHash>&& hashes,
    block::pHash&& stop) noexcept
    : Message(api, network, bitcoin::Command::getheaders)
    , version_(version)
    , payload_(std::move(hashes))
    , stop_(std::move(stop))
{
    init_hash();
}

Getheaders::Getheaders(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const bitcoin::ProtocolVersionUnsigned version,
    std::vector<block::pHash>&& hashes,
    block::pHash&& stop) noexcept
    : Message(api, std::move(header))
    , version_(version)
    , payload_(std::move(hashes))
    , stop_(std::move(stop))
{
}

auto Getheaders::payload() const noexcept -> OTData
{
    ProtocolVersionField version{version_};
    auto output = Data::Factory(&version, sizeof(version));
    output += Data::Factory(CompactSize(payload_.size()).Encode());

    for (const auto& hash : payload_) {
        if (32 == hash->size()) {
            output += hash;
        } else {
            LogOutput(OT_METHOD)(__func__)(": Invalid hash: ")(hash->asHex())
                .Flush();
        }
    }

    if (32 == stop_->size()) {
        output += stop_;
    } else {
        output += block::BlankHash();
    }

    return output;
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
