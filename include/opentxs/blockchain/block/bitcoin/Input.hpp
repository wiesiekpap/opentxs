// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_INPUT_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_INPUT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"

namespace opentxs
{
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
struct Outpoint {
    std::array<std::byte, 32> txid_{};
    std::array<std::byte, 4> index_{};

    auto operator<(const Outpoint& rhs) const noexcept -> bool;
    auto operator<=(const Outpoint& rhs) const noexcept -> bool;
    auto operator>(const Outpoint& rhs) const noexcept -> bool;
    auto operator>=(const Outpoint& rhs) const noexcept -> bool;
    auto operator==(const Outpoint& rhs) const noexcept -> bool;
    auto operator!=(const Outpoint& rhs) const noexcept -> bool;

    auto Bytes() const noexcept -> ReadView;
    auto Index() const noexcept -> std::uint32_t;
    auto Txid() const noexcept -> ReadView;
    auto str() const noexcept -> std::string;

    Outpoint(const ReadView serialized) noexcept(false);
    Outpoint(const ReadView txid, const std::uint32_t index) noexcept(false);
    Outpoint() noexcept;
    Outpoint(const Outpoint&) noexcept;
    Outpoint(Outpoint&&) noexcept;
    Outpoint& operator=(const Outpoint&) noexcept;
    Outpoint& operator=(Outpoint&&) noexcept;
};

class Input
{
public:
    using FilterType = Transaction::FilterType;
    using KeyID = api::client::blockchain::Key;
    using Match = Transaction::Match;
    using Matches = Transaction::Matches;
    using Patterns = Transaction::Patterns;
    using ParsedPatterns = Transaction::ParsedPatterns;
    using SerializeType = proto::BlockchainTransactionInput;

    OPENTXS_EXPORT virtual auto CalculateSize(
        const bool normalized = false) const noexcept -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto Coinbase() const noexcept -> Space = 0;
    OPENTXS_EXPORT virtual auto ExtractElements(
        const filter::Type style) const noexcept -> std::vector<Space> = 0;
    OPENTXS_EXPORT virtual auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    OPENTXS_EXPORT virtual auto GetPatterns() const noexcept
        -> std::vector<PatternID> = 0;
    OPENTXS_EXPORT virtual auto Keys() const noexcept -> std::vector<KeyID> = 0;
    OPENTXS_EXPORT virtual auto PreviousOutput() const noexcept
        -> const Outpoint& = 0;
    OPENTXS_EXPORT virtual auto Serialize(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;
    OPENTXS_EXPORT virtual auto Serialize(
        const api::client::Blockchain& blockchain,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto SerializeNormalized(
        const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    OPENTXS_EXPORT virtual auto Script() const noexcept
        -> const bitcoin::Script& = 0;
    OPENTXS_EXPORT virtual auto Sequence() const noexcept -> std::uint32_t = 0;
    OPENTXS_EXPORT virtual auto Witness() const noexcept
        -> const std::vector<Space>& = 0;

    virtual ~Input() = default;

protected:
    Input() noexcept = default;

private:
    Input(const Input&) = delete;
    Input(Input&&) = delete;
    Input& operator=(const Input&) = delete;
    Input& operator=(Input&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
