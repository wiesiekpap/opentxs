// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Reject.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Reject::"

namespace be = boost::endian;

namespace opentxs::factory
{
auto BitcoinP2PReject(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Reject*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Reject;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    auto expectedSize = sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Reject 1")
            .Flush();

        return nullptr;
    }

    auto* it{static_cast<const std::byte*>(payload)};
    // -----------------------------------------------
    auto messageSize = std::size_t{0};
    const bool decodedSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, messageSize);

    if (!decodedSize) {
        LogOutput(__func__)(": CompactSize incomplete for message field")
            .Flush();

        return nullptr;
    }
    // -----------------------------------------------
    expectedSize += messageSize;

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for message field")
            .Flush();

        return nullptr;
    }
    const std::string message{reinterpret_cast<const char*>(it), messageSize};
    it += messageSize;
    // -----------------------------------------------
    expectedSize += sizeof(std::uint8_t);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for code field")
            .Flush();

        return nullptr;
    }

    auto raw_code = be::little_uint8_buf_t{};
    std::memcpy(static_cast<void*>(&raw_code), it, sizeof(raw_code));
    it += sizeof(raw_code);
    const std::uint8_t code = raw_code.value();
    expectedSize += sizeof(std::byte);

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for Reject 1")
            .Flush();

        return nullptr;
    }
    // -----------------------------------------------
    std::size_t reasonSize{0};
    const bool decodedReasonSize = network::blockchain::bitcoin::DecodeSize(
        it, expectedSize, size, reasonSize);

    if (!decodedReasonSize) {
        LogOutput(__func__)(": CompactSize incomplete for reason field")
            .Flush();

        return nullptr;
    }
    // -----------------------------------------------
    expectedSize += reasonSize;

    if (expectedSize > size) {
        LogOutput("opentxs::factory::")(__func__)(
            ": Size below minimum for reason field")
            .Flush();

        return nullptr;
    }
    const std::string reason{reinterpret_cast<const char*>(it), reasonSize};
    it += reasonSize;
    // -----------------------------------------------
    // This next field is "sometimes there".
    // Sometimes it's there (or not) for a single code!
    // Because the code means different things depending on
    // what got rejected.
    //
    auto extra = Data::Factory();

    // better than hardcoding "32"
    expectedSize += sizeof(bitcoin::BlockHeaderHashField);

    if (expectedSize <= size) {
        extra->Concatenate(it, sizeof(bitcoin::BlockHeaderHashField));
        it += sizeof(bitcoin::BlockHeaderHashField);
    }
    // -----------------------------------------------
    try {
        return new ReturnType(
            api,
            std::move(pHeader),
            message,
            static_cast<bitcoin::RejectCode>(code),
            reason,
            extra);
    } catch (...) {
        LogOutput("opentxs::factory::")(__func__)(": Checksum failure").Flush();

        return nullptr;
    }
}

auto BitcoinP2PReject(
    const api::Core& api,
    const blockchain::Type network,
    const std::string& message,
    const std::uint8_t code,
    const std::string& reason,
    const Data& extra) -> blockchain::p2p::bitcoin::message::Reject*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Reject;

    return new ReturnType(
        api,
        network,
        message,
        static_cast<bitcoin::RejectCode>(code),
        reason,
        extra);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{

Reject::Reject(
    const api::Core& api,
    const blockchain::Type network,
    const std::string& message,
    const bitcoin::RejectCode code,
    const std::string& reason,
    const Data& extra) noexcept
    : Message(api, network, bitcoin::Command::reject)
    , message_(message)
    , code_(code)
    , reason_(reason)
    , extra_(Data::Factory(extra))
{
    init_hash();
}

Reject::Reject(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const std::string& message,
    const bitcoin::RejectCode code,
    const std::string& reason,
    const Data& extra) noexcept(false)
    : Message(api, std::move(header))
    , message_(message)
    , code_(code)
    , reason_(reason)
    , extra_(Data::Factory(extra))
{
    verify_checksum();
}

auto Reject::payload() const noexcept -> OTData
{
    auto output = Data::Factory(BitcoinString(message_));

    be::little_uint8_buf_t code(static_cast<std::uint8_t>(code_));
    output->Concatenate(&code, sizeof(code));

    output += Data::Factory(BitcoinString(reason_));

    if (false == extra_->empty()) { output += extra_; }

    return output;
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message
