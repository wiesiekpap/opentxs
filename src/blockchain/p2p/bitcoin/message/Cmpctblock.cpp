// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Cmpctblock.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Cmpctblock::"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PCmpctblock(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Cmpctblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    const auto raw_cmpctblock(Data::Factory(payload, size));
    // --------------------------------------------------------
    try {
        return new ReturnType(api, std::move(pHeader), raw_cmpctblock);
    } catch (...) {
        LogOutput("opentxs::factory::")(__func__)(": Checksum failure").Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PCmpctblock(
    const api::Core& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock)
    -> blockchain::p2p::bitcoin::message::Cmpctblock*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Cmpctblock;

    return new ReturnType(api, network, raw_cmpctblock);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
auto Cmpctblock::payload() const noexcept -> OTData
{
    try {
        return getRawCmpctblock();
    } catch (...) {
        return Data::Factory();
    }
}

// We have all the data members to create the message from scratch (for sending)
Cmpctblock::Cmpctblock(
    const api::Core& api,
    const blockchain::Type network,
    const Data& raw_cmpctblock) noexcept
    : Message(api, network, bitcoin::Command::cmpctblock)
    , raw_cmpctblock_(Data::Factory(raw_cmpctblock))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Cmpctblock::Cmpctblock(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const Data& raw_cmpctblock) noexcept(false)
    : Message(api, std::move(header))
    , raw_cmpctblock_(Data::Factory(raw_cmpctblock))
{
    verify_checksum();
}

}  // namespace  opentxs::blockchain::p2p::bitcoin::message
