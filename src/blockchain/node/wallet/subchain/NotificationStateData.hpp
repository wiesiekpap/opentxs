// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"

#pragma once

#include <cs_ordered_guarded.h>
#include <cs_shared_guarded.h>
#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <string_view>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "serialization/protobuf/HDPath.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
template <class T>
class shared_ptr;
}  // namespace boost

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace crypto
{

class Account;
class Notification;
}  // namespace crypto

namespace database
{
class Wallet;
}  // namespace database

namespace node
{
namespace internal
{
class Manager;
class Mempool;
}  // namespace internal

namespace wallet
{
class Accounts;
class Progress;
class Rescan;
class Scan;
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
class Push;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Log;
class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::wallet
{
class NotificationStateData final : public SubchainStateData
{
public:
    NotificationStateData(
        const api::Session& api,
        const node::internal::Manager& node,
        database::Wallet& db,
        const node::internal::Mempool& mempool,
        const cfilter::Type filter,
        const crypto::Subchain subchain,
        const network::zeromq::BatchID batch,
        const std::string_view parent,
        const opentxs::PaymentCode& code,
        const crypto::Notification& subaccount,
        allocator_type alloc) noexcept;

    ~NotificationStateData() final = default;

private:
    using PaymentCode =
        libguarded::shared_guarded<opentxs::PaymentCode, std::shared_mutex>;
    using Cache =
        libguarded::ordered_guarded<Vector<block::Position>, std::shared_mutex>;

    const proto::HDPath path_;
    const CString pc_display_;
    mutable PaymentCode code_;
    mutable Cache cache_;

    auto CheckCache(const std::size_t outstanding, FinishedCallback cb)
        const noexcept -> void final;
    auto do_startup() noexcept -> void final;
    auto get_index(const boost::shared_ptr<const SubchainStateData>& me)
        const noexcept -> Index final;
    auto handle_confirmed_matches(
        const bitcoin::block::Block& block,
        const block::Position& position,
        const block::Matches& confirmed,
        const Log& log) const noexcept -> void final;
    auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const bitcoin::block::Transaction> tx) const noexcept
        -> void final;
    auto init_keys() const noexcept -> OTPasswordPrompt;
    auto process(
        const block::Match& match,
        const bitcoin::block::Transaction& tx,
        const PasswordPrompt& reason) const noexcept -> void;
    auto process(
        const opentxs::PaymentCode& remote,
        const PasswordPrompt& reason) const noexcept -> void;

    auto init_contacts() noexcept -> void;
    auto work() noexcept -> int final;

    NotificationStateData() = delete;
    NotificationStateData(const NotificationStateData&) = delete;
    NotificationStateData(NotificationStateData&&) = delete;
    NotificationStateData& operator=(const NotificationStateData&) = delete;
    NotificationStateData& operator=(NotificationStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
