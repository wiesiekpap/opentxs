// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/p2p/bitcoin/message/Filteradd.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>
#include <vector>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

// #define OT_METHOD
// "opentxs::blockchain::p2p::bitcoin::message::implementation::Filteradd::"

namespace opentxs::factory
{
auto BitcoinP2PFilteradd(
    const api::Core& api,
    std::unique_ptr<blockchain::p2p::bitcoin::Header> pHeader,
    const blockchain::p2p::bitcoin::ProtocolVersion version,
    const void* payload,
    const std::size_t size)
    -> blockchain::p2p::bitcoin::message::internal::Filteradd*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filteradd;

    if (false == bool(pHeader)) {
        LogOutput("opentxs::factory::")(__func__)(": Invalid header").Flush();

        return nullptr;
    }

    return new ReturnType(
        api, std::move(pHeader), Data::Factory(payload, size));
}

auto BitcoinP2PFilteradd(
    const api::Core& api,
    const blockchain::Type network,
    const Data& element)
    -> blockchain::p2p::bitcoin::message::internal::Filteradd*
{
    namespace bitcoin = blockchain::p2p::bitcoin;
    using ReturnType = bitcoin::message::implementation::Filteradd;

    return new ReturnType(api, network, element);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
Filteradd::Filteradd(
    const api::Core& api,
    const blockchain::Type network,
    const Data& element) noexcept
    : Message(api, network, bitcoin::Command::filteradd)
    , element_(element)
{
    init_hash();
}

Filteradd::Filteradd(
    const api::Core& api,
    std::unique_ptr<Header> header,
    const Data& element) noexcept
    : Message(api, std::move(header))
    , element_(element)
{
}

auto Filteradd::payload() const noexcept -> OTData
{
    try {
        const auto size = CompactSize(element_->size()).Encode();
        auto output = Data::Factory(size.data(), size.size());
        output += element_;

        return output;
    } catch (...) {
        return Data::Factory();
    }
}
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
