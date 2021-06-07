// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_OUTPUT_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_OUTPUT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Script;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace proto
{
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
class Output
{
public:
    using ContactID = Transaction::ContactID;
    using FilterType = Transaction::FilterType;
    using ParsedPatterns = Transaction::ParsedPatterns;
    using Match = Transaction::Match;
    using Matches = Transaction::Matches;
    using KeyID = Transaction::KeyID;
    using KeyData = Transaction::KeyData;
    using SerializeType = proto::BlockchainTransactionOutput;

    OPENTXS_EXPORT virtual auto CalculateSize() const noexcept
        -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto ExtractElements(
        const filter::Type style) const noexcept -> std::vector<Space> = 0;
    OPENTXS_EXPORT virtual auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    OPENTXS_EXPORT virtual auto GetPatterns() const noexcept
        -> std::vector<PatternID> = 0;
    OPENTXS_EXPORT virtual auto Note(const api::client::Blockchain& blockchain)
        const noexcept -> std::string = 0;
    OPENTXS_EXPORT virtual auto Keys() const noexcept -> std::vector<KeyID> = 0;
    OPENTXS_EXPORT virtual auto Payee() const noexcept -> ContactID = 0;
    OPENTXS_EXPORT virtual auto Payer() const noexcept -> ContactID = 0;
    OPENTXS_EXPORT virtual auto Print() const noexcept -> std::string = 0;
    OPENTXS_EXPORT virtual auto Serialize(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;
    OPENTXS_EXPORT virtual auto Serialize(
        const api::client::Blockchain& blockchain,
        SerializeType& destination) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto Script() const noexcept
        -> const bitcoin::Script& = 0;
    OPENTXS_EXPORT virtual auto Value() const noexcept -> std::int64_t = 0;

    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    virtual ~Output() = default;

protected:
    Output() noexcept = default;

private:
    Output(const Output&) = delete;
    Output(Output&&) = delete;
    Output& operator=(const Output&) = delete;
    Output& operator=(Output&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
