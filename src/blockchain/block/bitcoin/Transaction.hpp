// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
class Contacts;
}  // namespace client

class Core;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
struct SigHash;
}  // namespace bitcoin
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Transaction final : public internal::Transaction
{
public:
    static const VersionNumber default_version_;

    auto AssociatedLocalNyms(const api::client::Blockchain& blockchain)
        const noexcept -> std::vector<OTNymID> final;
    auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        const api::client::Contacts& contacts,
        const identifier::Nym& nym) const noexcept
        -> std::vector<OTIdentifier> final;
    auto CalculateSize() const noexcept -> std::size_t final
    {
        return calculate_size(false);
    }
    auto Chains() const noexcept -> std::vector<blockchain::Type> final
    {
        return chains_;
    }
    auto clone() const noexcept -> std::unique_ptr<internal::Transaction> final
    {
        return std::make_unique<Transaction>(*this);
    }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const api::client::Blockchain& blockchain,
        const FilterType type,
        const Patterns& txos,
        const Patterns& elements) const noexcept -> Matches final;
    auto GetPatterns() const noexcept -> std::vector<PatternID> final;
    auto GetPreimageBTC(
        const std::size_t index,
        const blockchain::bitcoin::SigHash& hashType) const noexcept
        -> Space final;
    auto ID() const noexcept -> const Txid& final { return txid_; }
    auto IDNormalized() const noexcept -> const Identifier&;
    auto Inputs() const noexcept -> const bitcoin::Inputs& final
    {
        return *inputs_;
    }
    auto Locktime() const noexcept -> std::uint32_t final { return lock_time_; }
    auto Memo(const api::client::Blockchain& blockchain) const noexcept
        -> std::string final;
    auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount final;
    auto Outputs() const noexcept -> const bitcoin::Outputs& final
    {
        return *outputs_;
    }
    auto SegwitFlag() const noexcept -> std::byte final { return segwit_flag_; }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(const api::client::Blockchain& blockchain) const noexcept
        -> std::optional<SerializeType> final;
    auto Timestamp() const noexcept -> Time final { return time_; }
    auto Version() const noexcept -> std::int32_t final { return version_; }
    auto WTXID() const noexcept -> const Txid& final { return wtxid_; }

    auto AssociatePreviousOutput(
        const api::client::Blockchain& blockchain,
        const std::size_t index,
        const proto::BlockchainTransactionOutput& output) noexcept -> bool final
    {
        return inputs_->AssociatePreviousOutput(blockchain, index, output);
    }
    auto ForTestingOnlyAddKey(
        const std::size_t index,
        const api::client::blockchain::Key& key) noexcept -> bool final
    {
        return outputs_->ForTestingOnlyAddKey(index, key);
    }
    auto MergeMetadata(
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const SerializeType& rhs) noexcept -> void final;
    auto SetMemo(const std::string& memo) noexcept -> void final
    {
        memo_ = memo;
    }

    Transaction(
        const api::Core& api,
        const VersionNumber serializeVersion,
        const bool isGeneration,
        const std::int32_t version,
        const std::byte segwit,
        const std::uint32_t lockTime,
        const pTxid&& txid,
        const pTxid&& wtxid,
        const Time& time,
        const std::string& memo,
        std::unique_ptr<internal::Inputs> inputs,
        std::unique_ptr<internal::Outputs> outputs,
        std::vector<blockchain::Type>&& chains) noexcept(false);
    Transaction(const Transaction&) noexcept;

    ~Transaction() final = default;

private:
    const api::Core& api_;
    const VersionNumber serialize_version_;
    const bool is_generation_;
    const std::int32_t version_;
    const std::byte segwit_flag_;
    const std::uint32_t lock_time_;
    const pTxid txid_;
    const pTxid wtxid_;
    const Time time_;
    const std::unique_ptr<internal::Inputs> inputs_;
    const std::unique_ptr<internal::Outputs> outputs_;
    mutable std::optional<OTIdentifier> normalized_id_;
    mutable std::optional<std::size_t> size_;
    mutable std::optional<std::size_t> normalized_size_;
    std::string memo_;
    std::vector<blockchain::Type> chains_;

    static auto calculate_witness_size(const Space& witness) noexcept
        -> std::size_t;
    static auto calculate_witness_size(const std::vector<Space>&) noexcept
        -> std::size_t;

    auto calculate_size(const bool normalize) const noexcept -> std::size_t;
    auto calculate_witness_size() const noexcept -> std::size_t;
    auto serialize(const AllocateOutput destination, const bool normalize)
        const noexcept -> std::optional<std::size_t>;

    Transaction() = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
