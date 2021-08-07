// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace internal
{
struct Network;
struct WalletDatabase;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
}  // namespace socket

class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Accounts
{
public:
    auto Add(const identifier::Nym& nym) noexcept -> bool;
    auto Add(const network::zeromq::Frame& message) noexcept -> bool;
    auto Mempool(
        std::shared_ptr<const block::bitcoin::Transaction>&& tx) noexcept
        -> void;
    auto Reorg(const block::Position& parent) noexcept -> bool;

    auto shutdown() noexcept -> void;
    auto state_machine(bool enabled) noexcept -> bool;

    Accounts(
        const api::Core& api,
        const api::client::internal::Blockchain& crypto,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const Type chain,
        const SimpleCallback& taskFinished) noexcept;
    ~Accounts();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Accounts(const Accounts&) = delete;
    Accounts(Accounts&&) = delete;
    auto operator=(const Accounts&) -> Accounts& = delete;
    auto operator=(Accounts&&) -> Accounts& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
