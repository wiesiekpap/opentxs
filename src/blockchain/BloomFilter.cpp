// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "1_Internal.hpp"              // IWYU pragma: associated
#include "blockchain/BloomFilter.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/BloomFilter.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

namespace opentxs::factory
{
auto BloomFilter(
    const api::Core& api,
    const std::uint32_t tweak,
    const blockchain::BloomUpdateFlag update,
    const std::size_t targets,
    const double fpRate) -> blockchain::BloomFilter*
{
    using ReturnType = blockchain::implementation::BloomFilter;

    return new ReturnType(api, tweak, update, targets, fpRate);
}

auto BloomFilter(const api::Core& api, const Data& serialized)
    -> blockchain::BloomFilter*
{
    using ReturnType = blockchain::implementation::BloomFilter;
    blockchain::internal::SerializedBloomFilter raw{};

    if (sizeof(raw) > serialized.size()) {
        LogOutput("opentxs::factory::")(__func__)(": Input too short").Flush();

        return nullptr;
    }

    const auto filterSize = serialized.size() - sizeof(raw);
    auto filter = (0 == filterSize)
                      ? Data::Factory()
                      : Data::Factory(serialized.data(), filterSize);
    std::memcpy(
        reinterpret_cast<std::byte*>(&raw),
        static_cast<const std::byte*>(serialized.data()) + filterSize,
        sizeof(raw));

    return new ReturnType(
        api,
        raw.tweak_.value(),
        static_cast<blockchain::BloomUpdateFlag>(raw.flags_.value()),
        raw.function_count_.value(),
        filter);
}
}  // namespace opentxs::factory

namespace opentxs::blockchain::implementation
{
const std::size_t BloomFilter::max_filter_bytes_ = 36000;
const std::size_t BloomFilter::max_hash_function_count_ = 50;
const std::uint32_t BloomFilter::seed_{4221880213};  // 0xFBA4C795

BloomFilter::BloomFilter(
    const api::Core& api,
    const Tweak tweak,
    const BloomUpdateFlag update,
    const std::size_t functionCount,
    const Filter& data) noexcept
    : blockchain::BloomFilter()
    , api_(api)
    , tweak_(tweak)
    , flags_(update)
    , function_count_(functionCount)
    , filter_(data)
{
}

BloomFilter::BloomFilter(
    const api::Core& api,
    const Tweak tweak,
    const BloomUpdateFlag update,
    const std::size_t targets,
    const FalsePositiveRate rate) noexcept
    : BloomFilter(api, tweak, update, 0, Filter(std::size_t(64)))
{
    const auto pre_calc_filter_size = static_cast<std::size_t>(
        static_cast<std::size_t>(
            (-1) / std::pow(std::log(2), 2) * std::log(rate)) *
        targets);

    const auto ideal_filter_bytesize = static_cast<std::size_t>(std::max(
        std::size_t(1),
        (std::min(pre_calc_filter_size, BloomFilter::max_filter_bytes_ * 8) /
         8)));

    filter_.resize(ideal_filter_bytesize * 8u);

    // Optimal number of hash functions for given filter size and element count
    const auto precalc_hash_function_count = static_cast<std::size_t>(
        (ideal_filter_bytesize * 8u) /
        static_cast<std::size_t>(static_cast<double>(targets) * std::log(2)));

    function_count_ = static_cast<std::size_t>(std::max(
        std::size_t(1),
        std::min(
            precalc_hash_function_count,
            BloomFilter::max_hash_function_count_)));
}

BloomFilter::BloomFilter(
    const api::Core& api,
    const Tweak tweak,
    const BloomUpdateFlag update,
    const std::size_t functionCount,
    const Data& data) noexcept
    : BloomFilter(
          api,
          tweak,
          update,
          functionCount,
          Filter{
              static_cast<const std::uint8_t*>(data.data()),
              static_cast<const std::uint8_t*>(data.data()) + data.size()})
{
}

BloomFilter::BloomFilter(const BloomFilter& rhs) noexcept
    : BloomFilter(
          rhs.api_,
          rhs.tweak_,
          rhs.flags_,
          rhs.function_count_,
          rhs.filter_)
{
}

void BloomFilter::AddElement(const Data& in) noexcept
{
    const auto bitsize = filter_.size();

    for (std::size_t i{0}; i < function_count_; ++i) {
        const auto bit_index = hash(in, i) % bitsize;
        filter_.set(bit_index);
    }
}

auto BloomFilter::hash(const Data& input, std::size_t hash_index) const noexcept
    -> std::uint32_t
{
    OT_ASSERT(std::numeric_limits<std::uint32_t>::max() >= hash_index);

    auto seed = seed_ * static_cast<std::uint32_t>(hash_index);
    seed += tweak_;
    auto hash = std::uint32_t{};
    api_.Crypto().Hash().MurmurHash3_32(seed, input, hash);

    return hash;
}

auto BloomFilter::Serialize() const noexcept -> OTData
{
    std::vector<std::uint8_t> bytes{};
    boost::to_block_range(filter_, std::back_inserter(bytes));
    auto output = Data::Factory(bytes);
    blockchain::internal::SerializedBloomFilter raw{
        tweak_, flags_, function_count_};
    output->Concatenate(&raw, sizeof(raw));

    return output;
}

auto BloomFilter::Test(const Data& in) const noexcept -> bool
{
    const auto bitsize = filter_.size();

    for (std::size_t i{0}; i < function_count_; ++i) {
        const auto bit_index = hash(in, i) % bitsize;

        if (!filter_.test(bit_index)) { return false; }
    }

    return true;
}
}  // namespace opentxs::blockchain::implementation
