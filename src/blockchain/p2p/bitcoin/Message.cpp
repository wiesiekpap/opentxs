// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/Message.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/message/Cmpctblock.hpp"
#include "blockchain/p2p/bitcoin/message/Feefilter.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocks.hpp"
#include "blockchain/p2p/bitcoin/message/Getblocktxn.hpp"
#include "blockchain/p2p/bitcoin/message/Merkleblock.hpp"
#include "blockchain/p2p/bitcoin/message/Reject.hpp"
#include "blockchain/p2p/bitcoin/message/Sendcmpct.hpp"
#include "internal/blockchain/p2p/bitcoin/Factory.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_METHOD "opentxs::blockchain::p2p::bitcoin::Message::"

namespace opentxs::factory
{
auto BitcoinP2PMessage(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::Message*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::Message;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    ReturnType* pMessage{nullptr};

    switch (pHeader->Command()) {
        case bitcoin::Command::addr: {
            pMessage =
                BitcoinP2PAddr(api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::block: {
            pMessage = BitcoinP2PBlock(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::blocktxn: {
            pMessage = BitcoinP2PBlocktxn(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cmpctblock: {
            pMessage = BitcoinP2PCmpctblock(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::feefilter: {
            pMessage = BitcoinP2PFeefilter(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::filteradd: {
            pMessage = BitcoinP2PFilteradd(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::filterclear: {
            pMessage = BitcoinP2PFilterclear(api, std::move(pHeader));
        } break;
        case bitcoin::Command::filterload: {
            pMessage = BitcoinP2PFilterload(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getaddr: {
            pMessage = BitcoinP2PGetaddr(api, std::move(pHeader));
        } break;
        case bitcoin::Command::getblocks: {
            pMessage = BitcoinP2PGetblocks(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getblocktxn: {
            pMessage = BitcoinP2PGetblocktxn(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getdata: {
            pMessage = BitcoinP2PGetdata(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getheaders: {
            pMessage = BitcoinP2PGetheaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::headers: {
            pMessage = BitcoinP2PHeaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::inv: {
            pMessage =
                BitcoinP2PInv(api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::mempool: {
            pMessage = BitcoinP2PMempool(api, std::move(pHeader));
        } break;
        case bitcoin::Command::merkleblock: {
            pMessage = BitcoinP2PMerkleblock(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::notfound: {
            pMessage = BitcoinP2PNotfound(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::ping: {
            pMessage =
                BitcoinP2PPing(api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::pong: {
            pMessage =
                BitcoinP2PPong(api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::reject: {
            pMessage = BitcoinP2PReject(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::sendcmpct: {
            pMessage = BitcoinP2PSendcmpct(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::sendheaders: {
            pMessage = BitcoinP2PSendheaders(api, std::move(pHeader));
        } break;
        case bitcoin::Command::tx: {
            pMessage =
                BitcoinP2PTx(api, std::move(pHeader), version, payload, size)
                    .release();
        } break;
        case bitcoin::Command::verack: {
            pMessage = BitcoinP2PVerack(api, std::move(pHeader));
        } break;
        case bitcoin::Command::version: {
            pMessage = BitcoinP2PVersion(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getcfilters: {
            pMessage = BitcoinP2PGetcfilters(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cfilter: {
            pMessage = BitcoinP2PCfilter(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getcfheaders: {
            pMessage = BitcoinP2PGetcfheaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cfheaders: {
            pMessage = BitcoinP2PCfheaders(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::getcfcheckpt: {
            pMessage = BitcoinP2PGetcfcheckpt(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::cfcheckpt: {
            pMessage = BitcoinP2PCfcheckpt(
                api, std::move(pHeader), version, payload, size);
        } break;
        case bitcoin::Command::alert:
        case bitcoin::Command::checkorder:
        case bitcoin::Command::reply:
        case bitcoin::Command::submitorder:
        case bitcoin::Command::unknown:
        default: {
            LogOutput("opentxs::factory::")(__func__)(
                ": Unsupported message type")
                .Flush();
            return nullptr;
        }
    }

    return pMessage;
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin
{
auto Message::MaxPayload() -> std::size_t
{
    static_assert(
        std::numeric_limits<std::size_t>::max() >=
        std::numeric_limits<std::uint32_t>::max());

    return std::numeric_limits<std::uint32_t>::max();
}
}  // namespace opentxs::blockchain::p2p::bitcoin

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Message::Message(
    const api::Core& api,
    const blockchain::Type network,
    const bitcoin::Command command) noexcept
    : api_(api)
    , header_(std::make_unique<Header>(api, network, command))
{
    OT_ASSERT(header_);
}

Message::Message(const api::Core& api, std::unique_ptr<Header> header) noexcept
    : api_(api)
    , header_(std::move(header))
{
    OT_ASSERT(header_);
}

auto Message::Encode() const -> OTData
{
    OTData output = header_->Encode();
    output += payload();

    return output;
}

auto Message::calculate_checksum(const Data& payload) const noexcept -> OTData
{
    auto output = Data::Factory();
    P2PMessageHash(
        api_, header_->Network(), payload.Bytes(), output->WriteInto());

    return output;
}

auto Message::init_hash() noexcept -> void
{
    const auto data = payload();
    const auto size = data->size();
    header_->SetChecksum(size, calculate_checksum(data));
}

auto Message::verify_checksum() const noexcept(false) -> void
{
    const auto calculated = calculate_checksum(payload());
    const auto& header = header_->Checksum();

    if (header != calculated) {
        LogOutput(OT_METHOD)(__func__)(": Checksum failure").Flush();
        LogOutput("*  Calculated Payload:  ")(payload()->asHex()).Flush();
        LogOutput("*  Calculated Checksum: ")(calculated->asHex()).Flush();
        LogOutput("*  Provided Checksum:   ")(header.asHex()).Flush();

        throw std::runtime_error("checksum failure");
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
