// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/client/bitcoin/Network.hpp"  // IWYU pragma: associated

#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/client/Factory.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"  // IWYU pragma: keep

// #define OT_METHOD
// "opentxs::blockchain::client::bitcoin::implementation::Network::"

namespace opentxs::factory
{
auto BlockchainNetworkBitcoin(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const blockchain::Type type,
    const blockchain::client::internal::Config& config,
    const std::string& seednode,
    const std::string& syncEndpoint) noexcept
    -> std::unique_ptr<blockchain::client::internal::Network>
{
    using ReturnType = blockchain::client::bitcoin::implementation::Network;

    return std::make_unique<ReturnType>(
        api, blockchain, type, config, seednode, syncEndpoint);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::client::bitcoin::implementation
{
Network::Network(
    const api::Core& api,
    const api::client::internal::Blockchain& blockchain,
    const Type type,
    const internal::Config& config,
    const std::string& seednode,
    const std::string& syncEndpoint)
    : ot_super(api, blockchain, type, config, seednode, syncEndpoint)
{
    init();
}

auto Network::instantiate_header(const ReadView payload) const noexcept
    -> std::unique_ptr<block::Header>
{
    using Type = block::Header::SerializedType;

    return std::unique_ptr<block::Header>{
        factory::BitcoinBlockHeader(api_, proto::Factory<Type>(payload))};
}

Network::~Network() { Shutdown(); }
}  // namespace opentxs::blockchain::client::bitcoin::implementation
