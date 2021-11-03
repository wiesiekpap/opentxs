// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>

#include "blockchain/node/wallet/Actor.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
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

class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class Accounts final : public Actor
{
public:
    auto FinishBackgroundTasks() noexcept -> void final;
    auto ProcessBlockAvailable(const block::Hash& block) noexcept -> void final;
    auto ProcessKey() noexcept -> void final;
    auto ProcessMempool(
        std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void final;
    auto ProcessNewFilter(const block::Position& tip) noexcept -> void final;
    auto ProcessNym(const identifier::Nym& nym) noexcept -> bool;
    auto ProcessNym(const network::zeromq::Frame& message) noexcept -> bool;
    auto ProcessReorg(
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> bool final;
    auto ProcessStateMachine(bool enabled) noexcept -> bool final;
    auto ProcessTaskComplete(
        const Identifier& id,
        const char* type,
        bool enabled) noexcept -> void final;
    auto Shutdown() noexcept -> void final;

    Accounts(
        const api::Session& api,
        const api::crypto::Blockchain& crypto,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const Type chain,
        const std::function<void(const Identifier&, const char*)>&
            taskFinished) noexcept;

    ~Accounts() final;

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Accounts(const Accounts&) = delete;
    Accounts(Accounts&&) = delete;
    auto operator=(const Accounts&) -> Accounts& = delete;
    auto operator=(Accounts&&) -> Accounts& = delete;
};
}  // namespace opentxs::blockchain::node::wallet
