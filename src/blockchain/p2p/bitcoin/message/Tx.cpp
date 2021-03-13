// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Tx.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

//#define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Tx::"

namespace opentxs::factory
{
// We have a header and a raw payload. Parse it.
auto BitcoinP2PTx(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) -> blockchain::p2p::bitcoin::message::Tx*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Tx;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": Invalid header")
            .Flush();

        return nullptr;
    }

    try {
        return new ReturnType(
            api,
            std::move(pHeader),
            ReadView{static_cast<const char*>(payload), size});
    } catch (...) {
        LogOutput("opentxs::factory::")(__FUNCTION__)(": Checksum failure")
            .Flush();

        return nullptr;
    }
}

// We have all the data members to create the message from scratch (for sending)
auto BitcoinP2PTx(
    const api::Core& api,
    const blockchain::Type network,
    const ReadView transaction) -> blockchain::p2p::bitcoin::message::Tx*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::Tx;

    return new ReturnType(api, network, transaction);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{

auto Tx::payload() const noexcept -> OTData
{
    try {
        return getRawTx();
    } catch (...) {
        return Data::Factory();
    }
}

// We have all the data members to create the message from scratch (for sending)
Tx::Tx(
    const api::Core& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    : Message(api, network, bitcoin::Command::tx)
    , raw_tx_(api_.Factory().Data(transaction))
{
    init_hash();
}

// We have a header and the data members. They've been parsed, so now we are
// instantiating the message from them.
Tx::Tx(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const ReadView transaction) noexcept(false)
    : Message(api, std::move(header))
    , raw_tx_(api_.Factory().Data(transaction))
{
    verify_checksum();
}

}  // namespace  opentxs::blockchain::p2p::bitcoin::message
