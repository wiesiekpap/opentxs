// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "1_Internal.hpp"
#include "blockchain/block/Block.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Header;
}  // namespace internal

class Header;
class Transaction;
}  // namespace block
}  // namespace bitcoin
}  // namespace blockchain

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block::implementation
{
class Block : public block::Block,
              public blockchain::block::implementation::Block
{
public:
    using CalculatedSize =
        std::pair<std::size_t, network::blockchain::bitcoin::CompactSize>;
    using TxidIndex = UnallocatedVector<Space>;
    using TransactionMap = UnallocatedMap<ReadView, value_type>;

    static const std::size_t header_bytes_;

    template <typename HashType>
    static auto calculate_merkle_hash(
        const api::Session& api,
        const Type chain,
        const HashType& lhs,
        const HashType& rhs,
        AllocateOutput out) -> bool;
    template <typename InputContainer, typename OutputContainer>
    static auto calculate_merkle_row(
        const api::Session& api,
        const Type chain,
        const InputContainer& in,
        OutputContainer& out) -> bool;
    static auto calculate_merkle_value(
        const api::Session& api,
        const Type chain,
        const TxidIndex& txids) -> blockchain::block::Hash;

    auto at(const std::size_t index) const noexcept -> const value_type& final;
    auto at(const ReadView txid) const noexcept -> const value_type& final;
    auto begin() const noexcept -> const_iterator final { return cbegin(); }
    auto CalculateSize() const noexcept -> std::size_t final
    {
        return get_or_calculate_size().first;
    }
    auto cbegin() const noexcept -> const_iterator final
    {
        return const_iterator(this, 0);
    }
    auto cend() const noexcept -> const_iterator final
    {
        return const_iterator(this, index_.size());
    }
    auto end() const noexcept -> const_iterator final { return cend(); }
    auto ExtractElements(const cfilter::Type style) const noexcept
        -> Vector<Vector<std::byte>> final;
    auto FindMatches(
        const cfilter::Type type,
        const blockchain::block::Patterns& outpoints,
        const blockchain::block::Patterns& scripts,
        const Log& log) const noexcept -> blockchain::block::Matches final;
    auto Print() const noexcept -> UnallocatedCString override;
    auto Serialize(AllocateOutput bytes) const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final { return index_.size(); }

    Block(
        const api::Session& api,
        const blockchain::Type chain,
        std::unique_ptr<const blockchain::bitcoin::block::Header> header,
        TxidIndex&& index,
        TransactionMap&& transactions,
        std::optional<CalculatedSize>&& size = {}) noexcept(false);
    ~Block() override;

protected:
    using ByteIterator = std::byte*;

private:
    static const value_type null_tx_;

    const std::unique_ptr<const blockchain::bitcoin::block::Header> header_p_;
    const blockchain::bitcoin::block::Header& header_;
    const TxidIndex index_;
    const TransactionMap transactions_;
    mutable std::optional<CalculatedSize> size_;

    auto calculate_size() const noexcept -> CalculatedSize;
    virtual auto extra_bytes() const noexcept -> std::size_t { return 0; }
    auto get_or_calculate_size() const noexcept -> CalculatedSize;
    virtual auto serialize_post_header(ByteIterator& it, std::size_t& remaining)
        const noexcept -> bool;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::blockchain::bitcoin::block::implementation
