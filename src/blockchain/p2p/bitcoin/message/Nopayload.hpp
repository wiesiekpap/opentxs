// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <utility>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
template <typename InterfaceType>
class Nopayload final : virtual public InterfaceType,
                        public implementation::Message
{
public:
    Nopayload(
        const api::Core& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept
        : Message(api, network, command)
    {
        Message::init_hash();
    }
    Nopayload(const api::Core& api, std::unique_ptr<Header> header) noexcept
        : Message(api, std::move(header))
    {
    }

    ~Nopayload() final = default;

private:
    auto payload() const noexcept -> OTData final { return Data::Factory(); }

    Nopayload(const Nopayload&) = delete;
    Nopayload(Nopayload&&) = delete;
    auto operator=(const Nopayload&) -> Nopayload& = delete;
    auto operator=(Nopayload&&) -> Nopayload& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
