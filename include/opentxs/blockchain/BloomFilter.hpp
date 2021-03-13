// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOOMFILTER_HPP
#define OPENTXS_BLOCKCHAIN_BLOOMFILTER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

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
class BloomFilter
{
public:
    OPENTXS_EXPORT virtual OTData Serialize() const noexcept = 0;
    OPENTXS_EXPORT virtual bool Test(const Data& element) const noexcept = 0;

    OPENTXS_EXPORT virtual void AddElement(const Data& element) noexcept = 0;

    OPENTXS_EXPORT virtual ~BloomFilter() = default;

protected:
    BloomFilter() noexcept = default;

private:
    friend OTBloomFilter;

    virtual BloomFilter* clone() const noexcept = 0;

    BloomFilter(const BloomFilter& rhs) = delete;
    BloomFilter(BloomFilter&& rhs) = delete;
    BloomFilter& operator=(const BloomFilter& rhs) = delete;
    BloomFilter& operator=(BloomFilter&& rhs) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif
