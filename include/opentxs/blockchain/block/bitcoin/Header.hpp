// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_HEADER_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_HEADER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Header : virtual public block::Header
{
public:
    OPENTXS_EXPORT virtual auto MerkleRoot() const noexcept
        -> const block::Hash& = 0;
    OPENTXS_EXPORT virtual auto Encode() const noexcept -> OTData = 0;
    OPENTXS_EXPORT virtual auto Nonce() const noexcept -> std::uint32_t = 0;
    OPENTXS_EXPORT virtual auto nBits() const noexcept -> std::uint32_t = 0;
    OPENTXS_EXPORT virtual auto Timestamp() const noexcept -> Time = 0;
    OPENTXS_EXPORT virtual auto Version() const noexcept -> std::uint32_t = 0;

    OPENTXS_EXPORT ~Header() override = default;

protected:
    Header() noexcept = default;

private:
    Header(const Header&) = delete;
    Header(Header&&) = delete;
    Header& operator=(const Header&) = delete;
    Header& operator=(Header&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
