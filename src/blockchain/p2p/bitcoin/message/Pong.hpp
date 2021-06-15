// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
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
class Pong final : public internal::Pong, public implementation::Message
{
public:
    struct BitcoinFormat_60001 {
        NonceField nonce_{};

        BitcoinFormat_60001(const bitcoin::Nonce nonce) noexcept;
        BitcoinFormat_60001() noexcept;
    };

    auto Nonce() const noexcept -> bitcoin::Nonce final { return nonce_; }

    Pong(
        const api::Core& api,
        const blockchain::Type network,
        const bitcoin::Nonce nonce) noexcept;
    Pong(
        const api::Core& api,
        std::unique_ptr<Header> header,
        const bitcoin::Nonce nonce) noexcept;

    ~Pong() final = default;

private:
    const bitcoin::Nonce nonce_{};

    auto payload() const noexcept -> OTData final;

    Pong(const Pong&) = delete;
    Pong(Pong&&) = delete;
    auto operator=(const Pong&) -> Pong& = delete;
    auto operator=(Pong&&) -> Pong& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
