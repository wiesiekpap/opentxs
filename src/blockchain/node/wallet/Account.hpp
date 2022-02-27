// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <random>
#include <string_view>

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/BoostPMR.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"
#include "util/Actor.hpp"
#include "util/Gatekeeper.hpp"
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

namespace crypto
{
class Account;
class Deterministic;
}  // namespace crypto

namespace node
{
namespace internal
{
struct Network;
struct WalletDatabase;
}  // namespace internal

namespace wallet
{
class Accounts;
class Actor;
class DeterministicStateData;
class SubchainStateData;
}  // namespace wallet
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Push;
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Identifier;
class Outstanding;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Account::Imp final : public opentxs::Actor<Imp, AccountJobs>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;
    auto ProcessReorg(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        std::atomic_int& errors,
        const block::Position& parent) noexcept -> bool;
    auto Shutdown() noexcept -> void { signal_shutdown(); }

    Imp(Accounts& parent,
        const api::Session& api,
        const crypto::Account& account,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const filter::Type filter,
        const std::string_view shutdown,
        const std::string_view publish,
        const std::string_view pull,
        Outstanding&& jobs,
        allocator_type alloc) noexcept;

    ~Imp() final;

private:
    friend opentxs::Actor<Imp, AccountJobs>;

    enum class State {
        normal,
        reorg,
    };

    using Subchains = Map<OTIdentifier, DeterministicStateData>;
    using Subchain = node::internal::WalletDatabase::Subchain;
    using Direction = network::zeromq::socket::Direction;

    Accounts& parent_;
    const api::Session& api_;
    const crypto::Account& account_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const Type chain_;
    const filter::Type filter_type_;
    const std::function<void(const Identifier&, const char*)> task_finished_;
    network::zeromq::socket::Raw& to_parent_;
    State state_;
    std::default_random_engine rng_;
    Subchains internal_;
    Subchains external_;
    Subchains outgoing_;
    Subchains incoming_;
    Outstanding jobs_;

    auto do_shutdown() noexcept -> void;
    auto finish_background_tasks() noexcept -> void;
    template <typename Action>
    auto for_each_account(Action action) noexcept -> void
    {
        for (auto& [id, account] : internal_) { action(account); }

        for (auto& [id, account] : internal_) { action(account); }

        for (auto& [id, account] : outgoing_) { action(account); }

        for (auto& [id, account] : incoming_) { action(account); }
    }
    template <typename Action>
    auto for_each_subchain(Action action) noexcept -> void
    {
        auto buf = std::array<std::byte, 1024>{};
        auto alloc = alloc::BoostMonotonic{buf.data(), buf.size()};
        auto actors = Vector<wallet::Actor*>{&alloc};

        for (const auto& account : account_.GetHD()) {
            actors.emplace_back(&get(account, Subchain::Internal, internal_));
            actors.emplace_back(&get(account, Subchain::External, external_));
        }

        for (const auto& account : account_.GetPaymentCode()) {
            actors.emplace_back(&get(account, Subchain::Outgoing, outgoing_));
            actors.emplace_back(&get(account, Subchain::Incoming, incoming_));
        }

        std::shuffle(actors.begin(), actors.end(), rng_);

        for (auto* actor : actors) { action(*actor); }
    }
    auto get(
        const crypto::Deterministic& account,
        const Subchain subchain,
        Subchains& map) noexcept -> DeterministicStateData&;
    auto instantiate(
        const crypto::Deterministic& account,
        const Subchain subchain,
        Subchains& map) noexcept -> DeterministicStateData&;
    auto pipeline(const Work work, Message&& msg) noexcept -> void;
    auto process_block(Message&& in) noexcept -> void;
    auto process_block(const block::Hash& block) noexcept -> void;
    auto process_filter(Message&& in) noexcept -> void;
    auto process_filter(const block::Position& tip) noexcept -> void;
    auto process_key(Message&& in) noexcept -> void;
    auto process_mempool(Message&& in) noexcept -> void;
    auto process_mempool(
        std::shared_ptr<const block::bitcoin::Transaction> tx) noexcept -> void;
    auto process_job_finished(Message&& in) noexcept -> void;
    auto process_job_finished(const Identifier& id, const char* type) noexcept
        -> void;
    auto startup() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto work() noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::wallet
