// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_INPUT_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_INPUT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
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

struct Outpoint;
}  // namespace block
}  // namespace blockchain

namespace proto
{
class BlockchainTransactionInput;
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
class OPENTXS_EXPORT Input
{
public:
    using FilterType = Transaction::FilterType;
    using KeyID = Transaction::KeyID;
    using KeyData = Transaction::KeyData;
    using Match = Transaction::Match;
    using Matches = Transaction::Matches;
    using Patterns = Transaction::Patterns;
    using ParsedPatterns = Transaction::ParsedPatterns;
    using SerializeType = proto::BlockchainTransactionInput;

    virtual auto CalculateSize(const bool normalized = false) const noexcept
        -> std::size_t = 0;
    virtual auto Coinbase() const noexcept -> Space = 0;
    virtual auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> = 0;
    virtual auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    virtual auto GetPatterns() const noexcept -> std::vector<PatternID> = 0;
    virtual auto Keys() const noexcept -> std::vector<KeyID> = 0;
    virtual auto PreviousOutput() const noexcept -> const Outpoint& = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    virtual auto Serialize(
        const api::client::Blockchain& blockchain,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool = 0;
    virtual auto SerializeNormalized(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;
    virtual auto Script() const noexcept -> const bitcoin::Script& = 0;
    virtual auto Sequence() const noexcept -> std::uint32_t = 0;
    virtual auto Witness() const noexcept -> const std::vector<Space>& = 0;

    virtual auto SetKeyData(const KeyData& data) noexcept -> void = 0;

    virtual ~Input() = default;

protected:
    Input() noexcept = default;

private:
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
