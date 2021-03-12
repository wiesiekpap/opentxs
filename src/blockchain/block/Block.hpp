// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Header.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::blockchain::block::implementation
{
class Block : virtual public block::Block
{
public:
    auto Header() const noexcept -> const block::Header& override
    {
        return base_header_;
    }
    auto ID() const noexcept -> const block::Hash& final
    {
        return base_header_.Hash();
    }

protected:
    const api::Core& api_;

    Block(const api::Core& api, const block::Header& header) noexcept;

private:
    const block::Header& base_header_;

    Block() = delete;
    Block(const Block&) = delete;
    Block(Block&&) = delete;
    auto operator=(const Block&) -> Block& = delete;
    auto operator=(Block&&) -> Block& = delete;
};
}  // namespace opentxs::blockchain::block::implementation
