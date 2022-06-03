// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string_view>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "blockchain/node/wallet/spend/Proposals.hpp"
#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"
#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Account.hpp"
#include "internal/blockchain/node/wallet/Accounts.hpp"
#include "internal/blockchain/node/wallet/FeeOracle.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WorkType.hpp"
#include "serialization/protobuf/BlockchainTransactionOutput.pb.h"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
#include "serialization/protobuf/Enums.pb.h"
#include "util/JobCounter.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
namespace node
{
class Mempool;
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

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class Wallet final : virtual public node::internal::Wallet,
                     public Worker<api::Session>
{
public:
    auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx,
        std::promise<SendOutcome>&& promise) const noexcept -> void final;
    auto FeeEstimate() const noexcept -> std::optional<Amount> final
    {
        return fee_oracle_.EstimatedFee();
    }
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::pTxid> override;

    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetBalance(const identifier::Nym& owner, const Identifier& subaccount)
        const noexcept -> Balance final;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final;
    auto GetOutputs(alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(TxoState type, alloc::Resource* alloc = alloc::System())
        const noexcept -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& subaccount,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& subaccount,
        TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> final;
    auto GetOutputs(
        const crypto::Key& key,
        TxoState type,
        alloc::Resource* alloc = alloc::System()) const noexcept
        -> Vector<UTXO> final;
    auto GetTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<TxoTag> final;
    auto Height() const noexcept -> block::Height final;
    auto StartRescan() const noexcept -> bool final;

    auto Init() noexcept -> void final;
    auto Shutdown() noexcept -> void final
    {
        protect_shutdown([this] { shut_down(); });
    }

    Wallet(
        const api::Session& api,
        const node::internal::Network& parent,
        node::internal::WalletDatabase& db,
        const node::internal::Mempool& mempool,
        const Type chain,
        const std::string_view shutdown) noexcept;

    ~Wallet() final;

protected:
    auto pipeline(zmq::Message&& in) noexcept -> void final;
    auto state_machine() noexcept -> bool final;

private:
    auto shut_down() noexcept -> void;

private:
    using Work = wallet::WalletJobs;

    const node::internal::Network& parent_;
    node::internal::WalletDatabase& db_;
    const Type chain_;
    wallet::FeeOracle fee_oracle_;
    wallet::Accounts accounts_;
    wallet::Proposals proposals_;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
