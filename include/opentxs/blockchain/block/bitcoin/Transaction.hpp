// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_TRANSACTION_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_TRANSACTION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
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
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
namespace internal
{
struct Transaction;
}  // namespace internal

class Inputs;
class Outputs;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransaction;
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class OPENTXS_EXPORT Transaction
{
public:
    using FilterType = block::Block::FilterType;
    using Patterns = block::Block::Patterns;
    using ParsedPatterns = block::Block::ParsedPatterns;
    using Match = block::Block::Match;
    using Matches = block::Block::Matches;
    using KeyID = block::Block::KeyID;
    using ContactID = block::Block::ContactID;
    using KeyData = block::Block::KeyData;
    using SerializeType = proto::BlockchainTransaction;

    virtual auto AssociatedLocalNyms(const api::client::Blockchain& blockchain)
        const noexcept -> std::vector<OTNymID> = 0;
    virtual auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        const api::client::Contacts& contacts,
        const identifier::Nym& nym) const noexcept
        -> std::vector<OTIdentifier> = 0;
    virtual auto BlockPosition() const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto Chains() const noexcept -> std::vector<blockchain::Type> = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> = 0;
    virtual auto FindMatches(
        const FilterType type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetPatterns() const noexcept -> std::vector<PatternID> = 0;
    virtual auto ID() const noexcept -> const Txid& = 0;
    virtual auto IDNormalized() const noexcept -> const Identifier& = 0;
    virtual auto Inputs() const noexcept -> const bitcoin::Inputs& = 0;
    virtual auto Keys() const noexcept -> std::vector<KeyID> = 0;
    virtual auto Locktime() const noexcept -> std::uint32_t = 0;
    virtual auto Memo(const api::client::Blockchain& blockchain) const noexcept
        -> std::string = 0;
    virtual auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount = 0;
    virtual auto Outputs() const noexcept -> const bitcoin::Outputs& = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    virtual auto SegwitFlag() const noexcept -> std::byte = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(const api::client::Blockchain& blockchain)
        const noexcept -> std::optional<SerializeType> = 0;
    virtual auto Timestamp() const noexcept -> Time = 0;
    virtual auto Version() const noexcept -> std::int32_t = 0;
    virtual auto WTXID() const noexcept -> const Txid& = 0;
    virtual auto clone() const noexcept
        -> std::unique_ptr<internal::Transaction> = 0;

    virtual auto SetPosition(std::size_t position) noexcept -> void = 0;
    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    virtual ~Transaction() = default;

protected:
    Transaction() noexcept = default;

private:
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = delete;
    auto operator=(const Transaction&) -> Transaction& = delete;
    auto operator=(Transaction&&) -> Transaction& = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
