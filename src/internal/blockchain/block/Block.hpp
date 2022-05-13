// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::block::internal
{
struct Block : virtual public block::Block {
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto ExtractElements(const cfilter::Type style) const noexcept
        -> Vector<Vector<std::byte>> = 0;
    virtual auto FindMatches(
        const cfilter::Type type,
        const Patterns& txos,
        const Patterns& elements,
        const Log& log) const noexcept -> Matches = 0;

    ~Block() override = default;
};
}  // namespace opentxs::blockchain::block::internal
