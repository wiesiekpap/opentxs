// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <vector>

#include "internal/blockchain/client/Client.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

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

namespace database
{
namespace wallet
{
class Proposal;
class SubchainData;
class Transaction;
}  // namespace wallet
}  // namespace database
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::database::wallet
{
class Output
{
public:
    using AccountID = Identifier;
    using SubchainID = Identifier;
    using Parent = client::internal::WalletDatabase;
    using NodeID = Parent::NodeID;
    using Subchain = Parent::Subchain;
    using FilterType = Parent::FilterType;
    using UTXO = Parent::UTXO;
    using Spend = Parent::Spend;
    using State = client::Wallet::TxoState;

    auto CancelProposal(const Identifier& id) noexcept -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance;
    auto GetOutputs(State type) const noexcept -> std::vector<UTXO>;
    auto GetOutputs(const identifier::Nym& owner, State type) const noexcept
        -> std::vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        State type) const noexcept -> std::vector<UTXO>;
    auto GetMutex() const noexcept -> std::mutex&;
    auto GetUnspentOutputs() const noexcept -> std::vector<UTXO>;
    auto GetUnspentOutputs(const NodeID& balanceNode) const noexcept
        -> std::vector<UTXO>;

    auto AddConfirmedTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const std::size_t blockIndex,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) noexcept -> bool;
    auto AddOutgoingTransaction(
        const blockchain::Type chain,
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) noexcept -> bool;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        const Spend policy) noexcept -> std::optional<UTXO>;
    auto Rollback(
        const Lock& lock,
        const SubchainID& subchain,
        const block::Position& position) noexcept -> bool;

    Output(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const blockchain::Type chain,
        const wallet::SubchainData& subchains,
        wallet::Proposal& proposals,
        wallet::Transaction& transactions) noexcept;

    ~Output();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::blockchain::database::wallet
