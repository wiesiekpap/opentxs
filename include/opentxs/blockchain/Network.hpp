// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_NETWORK_HPP
#define OPENTXS_BLOCKCHAIN_NETWORK_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <future>

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block

namespace client
{
class BlockOracle;
class FilterOracle;
class HeaderOracle;
}  // namespace client
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class Network
{
public:
    using PendingOutgoing = std::future<block::pTxid>;

    OPENTXS_EXPORT virtual auto AddBlock(
        const std::shared_ptr<const block::bitcoin::Block> block) const noexcept
        -> bool = 0;
    OPENTXS_EXPORT virtual auto AddPeer(
        const p2p::Address& address) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto BlockOracle() const noexcept
        -> const client::BlockOracle& = 0;
    OPENTXS_EXPORT virtual auto FilterOracle() const noexcept
        -> const client::FilterOracle& = 0;
    OPENTXS_EXPORT virtual auto GetBalance() const noexcept -> Balance = 0;
    OPENTXS_EXPORT virtual auto GetBalance(
        const identifier::Nym& owner) const noexcept -> Balance = 0;
    OPENTXS_EXPORT virtual auto GetConfirmations(
        const std::string& txid) const noexcept -> ChainHeight = 0;
    OPENTXS_EXPORT virtual auto GetHeight() const noexcept -> ChainHeight = 0;
    OPENTXS_EXPORT virtual auto GetPeerCount() const noexcept
        -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto GetType() const noexcept -> Type = 0;
    OPENTXS_EXPORT virtual auto HeaderOracle() const noexcept
        -> const client::HeaderOracle& = 0;
    OPENTXS_EXPORT virtual auto Listen(
        const p2p::Address& address) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto SendToAddress(
        const identifier::Nym& sender,
        const std::string& address,
        const Amount amount,
        const std::string& memo = {}) const noexcept -> PendingOutgoing = 0;

    OPENTXS_EXPORT virtual auto Connect() noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto Disconnect() noexcept -> bool = 0;

    virtual ~Network() = default;

protected:
    Network() noexcept = default;

private:
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    Network& operator=(const Network&) = delete;
    Network& operator=(Network&&) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_NETWORK_HPP
