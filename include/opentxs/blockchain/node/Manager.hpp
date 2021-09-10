// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_NETWORK_HPP
#define OPENTXS_BLOCKCHAIN_NETWORK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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

namespace node
{
namespace internal
{
struct Network;
}  // namespace internal

class BlockOracle;
class FilterOracle;
class HeaderOracle;
class Wallet;
}  // namespace node

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace node
{
class OPENTXS_EXPORT Manager
{
public:
    using PendingOutgoing = std::future<SendOutcome>;

    virtual auto AddBlock(
        const std::shared_ptr<const block::bitcoin::Block> block) const noexcept
        -> bool = 0;
    virtual auto AddPeer(const p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto BlockOracle() const noexcept -> const node::BlockOracle& = 0;
    virtual auto FilterOracle() const noexcept -> const node::FilterOracle& = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetConfirmations(const std::string& txid) const noexcept
        -> ChainHeight = 0;
    virtual auto GetHeight() const noexcept -> ChainHeight = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto GetType() const noexcept -> Type = 0;
    virtual auto GetVerifiedPeerCount() const noexcept -> std::size_t = 0;
    virtual auto HeaderOracle() const noexcept -> const node::HeaderOracle& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Network& = 0;
    virtual auto Listen(const p2p::Address& address) const noexcept -> bool = 0;
    virtual auto SendToAddress(
        const identifier::Nym& sender,
        const std::string& address,
        const Amount amount,
        const std::string& memo = {}) const noexcept -> PendingOutgoing = 0;
    virtual auto SendToPaymentCode(
        const identifier::Nym& sender,
        const std::string& recipient,
        const Amount amount,
        const std::string& memo = {}) const noexcept -> PendingOutgoing = 0;
    virtual auto SendToPaymentCode(
        const identifier::Nym& sender,
        const PaymentCode& recipient,
        const Amount amount,
        const std::string& memo = {}) const noexcept -> PendingOutgoing = 0;
    virtual auto SyncTip() const noexcept -> block::Position = 0;
    virtual auto Wallet() const noexcept -> const node::Wallet& = 0;

    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Disconnect() noexcept -> bool = 0;

    virtual ~Manager() = default;

protected:
    Manager() noexcept = default;

private:
    Manager(const Manager&) = delete;
    Manager(Manager&&) = delete;
    auto operator=(const Manager&) -> Manager& = delete;
    auto operator=(Manager&&) -> Manager& = delete;
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_NETWORK_HPP
