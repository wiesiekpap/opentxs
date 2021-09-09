// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOOMFILTER_HPP
#define OPENTXS_BLOCKCHAIN_BLOOMFILTER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace blockchain
{
class BloomFilter;
}  // namespace blockchain

using OTBloomFilter = Pimpl<blockchain::BloomFilter>;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class OPENTXS_EXPORT BloomFilter
{
public:
    virtual auto Serialize() const noexcept -> OTData = 0;
    virtual auto Test(const Data& element) const noexcept -> bool = 0;

    virtual void AddElement(const Data& element) noexcept = 0;

    virtual ~BloomFilter() = default;

protected:
    BloomFilter() noexcept = default;

private:
    friend OTBloomFilter;

    virtual auto clone() const noexcept -> BloomFilter* = 0;

    BloomFilter(const BloomFilter& rhs) = delete;
    BloomFilter(BloomFilter&& rhs) = delete;
    auto operator=(const BloomFilter& rhs) -> BloomFilter& = delete;
    auto operator=(BloomFilter&& rhs) -> BloomFilter& = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif
