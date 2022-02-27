// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocated.hpp"
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
}  // namespace zeromq
}  // namespace network

class Identifier;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class Accounts::Imp final : public opentxs::Actor<Imp, AccountsJobs>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;
    auto Shutdown() noexcept -> void { signal_shutdown(); }

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
    friend opentxs::Actor<Imp, AccountsJobs>;

    enum class State {
        normal,
        reorg,
        post_reorg,
    };

    struct ReorgData {
        const block::Position ancestor_;
        const block::Position tip_;
        const std::size_t target_;
        std::size_t ready_;
        std::size_t done_;

        ReorgData(
            block::Position&& ancestor,
            block::Position&& tip,
            std::size_t target) noexcept
            : ancestor_(std::move(ancestor))
            , tip_(std::move(tip))
            , target_(target)
            , ready_(0)
            , done_(0)
        {
        }
    };

    using Direction = network::zeromq::socket::Direction;
    using AccountMap = Map<OTNymID, wallet::Account>;
    using NotificationMap = Map<OTIdentifier, NotificationStateData>;

    Accounts& parent_;
    const api::Session& api_;
    const node::internal::Network& node_;
    const node::internal::WalletDatabase& db_;
    const node::internal::Mempool& mempool_;
    const std::function<void(const Identifier&, const char*)> task_finished_;
    const Type chain_;
    const filter::Type filter_type_;
    const CString shutdown_endpoint_;
    const CString publish_endpoint_;
    const CString pull_endpoint_;
    network::zeromq::socket::Raw& to_parent_;
    network::zeromq::socket::Raw& to_accounts_;
    State state_;
    JobCounter job_counter_;
    Outstanding pc_counter_;
    std::optional<ReorgData> reorg_;
    AccountMap accounts_;
    NotificationMap notification_channels_;

    auto reorg_children() const noexcept -> std::size_t;

    auto do_reorg() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto finish_background_tasks() noexcept -> void;
    auto finish_reorg() noexcept -> void;
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
    auto process_nym(Message&& in) noexcept -> bool;
    auto process_nym(const identifier::Nym& nym) noexcept -> bool;
    auto process_reorg_ready(Message&& in) noexcept -> void;
    auto startup() noexcept -> void;
    auto state_normal(const Work work, Message&& msg) noexcept -> void;
    auto state_post_reorg(const Work work, Message&& msg) noexcept -> void;
    auto state_reorg(const Work work, Message&& msg) noexcept -> void;
    auto transition_state_normal(Message&& in) noexcept -> void;
    auto transition_state_post_reorg(Message&& in) noexcept -> void;
    auto transition_state_reorg(Message&& in) noexcept -> void;
    auto work() noexcept -> bool;

    Imp(Accounts& parent,
        const api::Session& api,
        const node::internal::Network& node,
        const node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const network::zeromq::BatchID batch,
        const Type chain,
        const std::string_view shutdown,
        const std::string_view endpoint,
        CString&& publish,
        CString&& pull,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::blockchain::node::wallet
