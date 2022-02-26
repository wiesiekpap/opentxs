// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string_view>

#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
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
struct Mempool;
struct Network;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Account;
class Actor;
class NotificationStateData;
}  // namespace wallet
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
class Raw;
}  // namespace socket

class Frame;
}  // namespace zeromq
}  // namespace network

class Identifier;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Accounts::Imp final : public opentxs::Actor<Imp, WalletJobs>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;
    auto Shutdown() noexcept -> void;

    Imp(Accounts& parent,
        const api::Session& api,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const std::string_view shutdown,
        const std::string_view endpoint,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend opentxs::Actor<Imp, WalletJobs>;

    using AccountMap = Map<OTNymID, wallet::Account>;
    using PCMap = Map<OTIdentifier, NotificationStateData>;

    Accounts& parent_;
    const api::Session& api_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const std::function<void(const Identifier&, const char*)> task_finished_;
    const Type chain_;
    const filter::Type filter_type_;
    network::zeromq::socket::Raw& to_wallet_;
    std::default_random_engine rng_;
    JobCounter job_counter_;
    AccountMap map_;
    Outstanding pc_counter_;
    PCMap payment_codes_;

    auto do_shutdown() noexcept -> void;
    auto finish_background_tasks() noexcept -> void;
    auto for_each(std::function<void(wallet::Actor&)> action) noexcept -> void;
    auto index_nym(const identifier::Nym& id) noexcept -> void;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block(Message&& in) noexcept -> void;
    auto process_block(const block::Hash& block) noexcept -> void;
    auto process_block_header(Message&& in) noexcept -> void;
    auto process_filter(Message&& in) noexcept -> void;
    auto process_filter(const block::Position& tip) noexcept -> void;
    auto process_job_finished(Message&& in) noexcept -> void;
    auto process_job_finished(const Identifier& id, const char* type) noexcept
        -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto process_mempool(Message&& in) noexcept -> void;
    auto process_mempool(
        std::shared_ptr<const block::bitcoin::Transaction>&& tx) noexcept
        -> void;
    auto process_nym(const identifier::Nym& nym) noexcept -> bool;
    auto process_nym(const network::zeromq::Frame& message) noexcept -> bool;
    auto process_reorg(Message&& in) noexcept -> void;
    auto process_reorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> bool;
    auto startup() noexcept -> void;
    auto state_machine() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::wallet
