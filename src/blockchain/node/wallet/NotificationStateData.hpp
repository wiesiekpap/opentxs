// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <functional>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

#include "blockchain/node/wallet/Index.hpp"
#include "blockchain/node/wallet/SubchainStateData.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/protobuf/HDPath.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class Deterministic;
}  // namespace blockchain

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
class Block;
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace internal
{
struct Network;
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

class Outstanding;
class PaymentCode;
}  // namespace opentxs

namespace opentxs::blockchain::node::wallet
{
class NotificationStateData final : public SubchainStateData
{
public:
    static auto calculate_id(
        const api::Core& api,
        const Type chain,
        const PaymentCode& code) noexcept -> OTIdentifier;

    NotificationStateData(
        const api::Core& api,
        const api::client::internal::Blockchain& crypto,
        const node::internal::Network& node,
        Accounts& parent,
        const WalletDatabase& db,
        const std::function<void(const Identifier&, const char*)>& taskFinished,
        Outstanding& jobCounter,
        const filter::Type filter,
        const Type chain,
        const identifier::Nym& nym,
        OTPaymentCode&& code,
        proto::HDPath&& path) noexcept;

    ~NotificationStateData() final = default;

private:
    class Index final : public wallet::Index
    {
    public:
        OTPaymentCode code_;

        auto Do(std::optional<Bip32Index> current, Bip32Index target) noexcept
            -> void final;

        Index(
            SubchainStateData& parent,
            Scan& scan,
            Rescan& rescan,
            Progress& progress,
            OTPaymentCode&& code) noexcept;

        ~Index() final = default;

    private:
        auto need_index(const std::optional<Bip32Index>& current) const noexcept
            -> std::optional<Bip32Index> final;
    };

    const proto::HDPath path_;
    Index index_;
    PaymentCode& code_;

    auto type() const noexcept -> std::stringstream final;

    auto get_index() noexcept -> Index& final { return index_; }
    auto handle_confirmed_matches(
        const block::bitcoin::Block& block,
        const block::Position& position,
        const block::Matches& confirmed) noexcept -> void final;
    auto handle_mempool_matches(
        const block::Matches& matches,
        std::unique_ptr<const block::bitcoin::Transaction> tx) noexcept
        -> void final;
    auto init_keys() noexcept -> OTPasswordPrompt;
    auto process(
        const block::Match match,
        const block::bitcoin::Transaction& tx,
        const PasswordPrompt& reason) noexcept -> void;

    NotificationStateData() = delete;
    NotificationStateData(const NotificationStateData&) = delete;
    NotificationStateData(NotificationStateData&&) = delete;
    NotificationStateData& operator=(const NotificationStateData&) = delete;
    NotificationStateData& operator=(NotificationStateData&&) = delete;
};
}  // namespace opentxs::blockchain::node::wallet
