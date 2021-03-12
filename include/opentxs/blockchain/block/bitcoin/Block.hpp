// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_BLOCK_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_BLOCK_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block : virtual public block::Block
{
public:
    using value_type = std::shared_ptr<const Transaction>;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Block, const value_type>;

    OPENTXS_EXPORT virtual auto at(const std::size_t index) const noexcept
        -> const value_type& = 0;
    OPENTXS_EXPORT virtual auto at(const ReadView txid) const noexcept
        -> const value_type& = 0;
    OPENTXS_EXPORT virtual auto begin() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto cbegin() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto cend() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto end() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto size() const noexcept -> std::size_t = 0;

    ~Block() override = default;

protected:
    Block() noexcept = default;

private:
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    Block& operator=(const Block&) = delete;
    Block& operator=(Block&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
