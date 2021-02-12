// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_INPUTS_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_INPUTS_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Inputs
{
public:
    using value_type = Input;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Inputs, const value_type>;
    using FilterType = Transaction::FilterType;
    using Patterns = Transaction::Patterns;
    using ParsedPatterns = Transaction::ParsedPatterns;
    using Match = Transaction::Match;
    using Matches = Transaction::Matches;

    OPENTXS_EXPORT virtual auto at(const std::size_t position) const
        noexcept(false) -> const value_type& = 0;
    OPENTXS_EXPORT virtual auto begin() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto CalculateSize(
        const bool normalized = false) const noexcept -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto cbegin() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto cend() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto end() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto ExtractElements(
        const filter::Type style) const noexcept -> std::vector<Space> = 0;
    OPENTXS_EXPORT virtual auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches = 0;
    OPENTXS_EXPORT virtual auto GetPatterns() const noexcept
        -> std::vector<PatternID> = 0;
    OPENTXS_EXPORT virtual auto Serialize(const AllocateOutput destination)
        const noexcept -> std::optional<std::size_t> = 0;
    OPENTXS_EXPORT virtual auto Serialize(
        const api::client::Blockchain& blockchain,
        proto::BlockchainTransaction& destination) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto SerializeNormalized(
        const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> = 0;
    OPENTXS_EXPORT virtual auto size() const noexcept -> std::size_t = 0;

    virtual auto at(const std::size_t position) noexcept(false)
        -> value_type& = 0;

    virtual ~Inputs() = default;

protected:
    Inputs() noexcept = default;

private:
    Inputs(const Inputs&) = delete;
    Inputs(Inputs&&) = delete;
    Inputs& operator=(const Inputs&) = delete;
    Inputs& operator=(Inputs&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
