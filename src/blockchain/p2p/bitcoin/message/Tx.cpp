// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Tx.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"  // IWYU pragma: keep
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

// #define OT_METHOD " opentxs::blockchain::p2p::bitcoin::message::Tx::"

namespace opentxs::factory
{
using ReturnType = blockchain::p2p::bitcoin::message::Tx;

auto BitcoinP2PTx(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>
{
    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    try {
        return std::make_unique<ReturnType>(
            api,
            std::move(pHeader),
            ReadView{static_cast<const char*>(payload), size});
    } catch (...) {
        LogOutput("opentxs::factory::")(__func__)(": Checksum failure").Flush();

        return nullptr;
    }
}

auto BitcoinP2PTx(
    const api::Core& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    -> std::unique_ptr<blockchain::p2p::bitcoin::message::internal::Tx>
{
    return std::make_unique<ReturnType>(api, network, transaction);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message
{
Tx::Tx(
    const api::Core& api,
    const blockchain::Type network,
    const ReadView transaction) noexcept
    : Message(api, network, bitcoin::Command::tx)
    , payload_(api_.Factory().Data(transaction))
{
    init_hash();
}

Tx::Tx(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const ReadView transaction) noexcept(false)
    : Message(api, std::move(header))
    , payload_(api_.Factory().Data(transaction))
{
    verify_checksum();
}

auto Tx::Transaction() const noexcept
    -> std::unique_ptr<const block::bitcoin::Transaction>
{
    return api_.Factory().BitcoinTransaction(
        header_->Network(), payload_->Bytes(), false);
}
}  // namespace  opentxs::blockchain::p2p::bitcoin::message
