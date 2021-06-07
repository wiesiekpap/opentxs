// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <new>
#include <set>
#include <vector>

#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::message
{
class Getblocktxn final : public implementation::Message
{
public:
    auto getBlockHash() const noexcept -> OTData
    {
        return Data::Factory(block_hash_);
    }
    auto getIndices() const noexcept -> const std::vector<std::size_t>&
    {
        return txn_indices_;
    }

    Getblocktxn(
        const api::Core& api,
        const blockchain::Type network,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices) noexcept;
    Getblocktxn(
        const api::Core& api,
        std::unique_ptr<Header> header,
        const Data& block_hash,
        const std::vector<std::size_t>& txn_indices) noexcept(false);

    ~Getblocktxn() final = default;

private:
    const OTData block_hash_;
    const std::vector<std::size_t> txn_indices_;

    auto payload() const noexcept -> OTData final;

    Getblocktxn(const Getblocktxn&) = delete;
    Getblocktxn(Getblocktxn&&) = delete;
    auto operator=(const Getblocktxn&) -> Getblocktxn& = delete;
    auto operator=(Getblocktxn&&) -> Getblocktxn& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message
