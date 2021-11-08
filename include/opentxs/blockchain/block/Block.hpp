// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace internal
{
struct Block;
}  // namespace internal

class Header;
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
class OPENTXS_EXPORT Block
{
public:
    virtual auto Header() const noexcept -> const block::Header& = 0;
    virtual auto ID() const noexcept -> const block::Hash& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Block& = 0;
    virtual auto Print() const noexcept -> std::string = 0;
    virtual auto Serialize(AllocateOutput bytes) const noexcept -> bool = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept -> internal::Block& = 0;

    virtual ~Block() = default;

protected:
    Block() noexcept = default;

private:
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
