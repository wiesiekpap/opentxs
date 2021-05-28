// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_BLOCKORACLE_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_BLOCKORACLE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace blockchain
{
namespace client
{
class BlockOracle
{
public:
    using BitcoinBlock = block::bitcoin::Block;
    using BitcoinBlock_p = std::shared_ptr<const BitcoinBlock>;
    using BitcoinBlockFuture = std::shared_future<BitcoinBlock_p>;
    using BlockHashes = std::vector<block::pHash>;
    using BitcoinBlockFutures = std::vector<BitcoinBlockFuture>;

    OPENTXS_EXPORT virtual auto DownloadQueue() const noexcept
        -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto LoadBitcoin(
        const block::Hash& block) const noexcept -> BitcoinBlockFuture = 0;
    OPENTXS_EXPORT virtual auto LoadBitcoin(
        const BlockHashes& hashes) const noexcept -> BitcoinBlockFutures = 0;
    OPENTXS_EXPORT virtual auto Validate(
        const BitcoinBlock& block) const noexcept -> bool = 0;

    virtual ~BlockOracle() = default;

protected:
    BlockOracle() noexcept = default;

private:
    BlockOracle(const BlockOracle&) = delete;
    BlockOracle(BlockOracle&&) = delete;
    BlockOracle& operator=(const BlockOracle&) = delete;
    BlockOracle& operator=(BlockOracle&&) = delete;
};
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs
#endif
