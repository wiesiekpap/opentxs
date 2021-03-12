// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "blockchain/bitcoin/CompactSize.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <string>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"

#define OT_COMPACT_SIZE_THRESHOLD_1 252
#define OT_COMPACT_SIZE_THRESHOLD_3 65535
#define OT_COMPACT_SIZE_THRESHOLD_5 4294967295

#define OT_COMPACT_SIZE_PREFIX_3 0xfd
#define OT_COMPACT_SIZE_PREFIX_5 0xfe
#define OT_COMPACT_SIZE_PREFIX_9 0xff

#define OT_METHOD "opentxs::blockchain::bitcoin::CompactSize::"

namespace be = boost::endian;

namespace opentxs::blockchain::bitcoin
{
auto DecodeCompactSizeFromPayload(
    ByteIterator& it,
    std::size_t& expected,
    const std::size_t size,
    std::size_t& output) noexcept -> bool
{
    auto cs = CompactSize{};
    auto ret = DecodeCompactSizeFromPayload(it, expected, size, cs);
    output = cs.Value();

    return ret;
}

auto DecodeCompactSizeFromPayload(
    ByteIterator& it,
    std::size_t& expected,
    const std::size_t size,
    std::size_t& output,
    std::size_t& csExtraBytes) noexcept -> bool
{
    auto cs = CompactSize{};
    auto ret = DecodeCompactSizeFromPayload(it, expected, size, cs);
    output = cs.Value();
    csExtraBytes = cs.Size() - 1;

    return ret;
}

auto DecodeCompactSizeFromPayload(
    ByteIterator& it,
    std::size_t& expectedSize,
    const std::size_t size,
    CompactSize& output) noexcept -> bool
{
    if (std::byte{0} == *it) {
        output = CompactSize{0};
        std::advance(it, 1);
    } else {
        auto csExtraBytes = CompactSize::CalculateSize(*it);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
        // std::size_t might be 32 bit
        if (sizeof(std::size_t) < csExtraBytes) {
            LogOutput("opentxs::blockchain::bitcoin::")(__FUNCTION__)(
                ": Size too big")
                .Flush();

            return false;
        }
#pragma GCC diagnostic pop

        expectedSize += csExtraBytes;

        if (expectedSize > size) { return false; }

        if (0 == csExtraBytes) {
            output = CompactSize{std::to_integer<std::uint8_t>(*it)};
            std::advance(it, 1);
        } else {
            std::advance(it, 1);
            output = CompactSize(Space{it, it + csExtraBytes}).Value();
            std::advance(it, csExtraBytes);
        }
    }

    return true;
}

CompactSize::CompactSize(std::uint64_t value) noexcept
    : data_(value)
{
    static_assert(sizeof(data_) >= sizeof(std::size_t));
}

CompactSize::CompactSize(const Bytes& bytes) noexcept(false)
    : data_(0)
{
    if (false == Decode(bytes)) {
        throw std::invalid_argument(
            "Wrong number of bytes: " + std::to_string(bytes.size()));
    }
}

auto CompactSize::operator=(const std::uint64_t rhs) noexcept -> CompactSize&
{
    data_ = rhs;

    return *this;
}

auto CompactSize::CalculateSize(const std::byte first) noexcept -> std::uint64_t
{
    auto marker{reinterpret_cast<const uint8_t&>(first)};

    if (OT_COMPACT_SIZE_PREFIX_9 == marker) {
        return 8;
    } else if (OT_COMPACT_SIZE_PREFIX_5 == marker) {
        return 4;
    } else if (OT_COMPACT_SIZE_PREFIX_3 == marker) {
        return 2;
    } else {
        return 0;
    }
}

template <typename SizeType>
void CompactSize::convert_from_raw(const std::vector<std::byte>& bytes) noexcept
{
    SizeType value{0};
    std::memcpy(&value, bytes.data(), sizeof(value));
    be::little_to_native_inplace(value);
    data_ = value;
}

template <typename SizeType>
auto CompactSize::convert_to_raw(AllocateOutput output) const noexcept -> bool
{
    OT_ASSERT(std::numeric_limits<SizeType>::max() >= data_);

    if (false == bool(output)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    const auto out = output(sizeof(SizeType));

    if (false == out.valid(sizeof(SizeType))) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output")
            .Flush();

        return false;
    }

    auto value{static_cast<SizeType>(data_)};
    be::native_to_little_inplace(value);
    std::memcpy(out.data(), &value, sizeof(value));

    return true;
}

auto CompactSize::Decode(const std::vector<std::byte>& bytes) noexcept -> bool
{
    bool output{true};

    if (sizeof(std::uint8_t) == bytes.size()) {
        convert_from_raw<std::uint8_t>(bytes);
    } else if (sizeof(std::uint16_t) == bytes.size()) {
        convert_from_raw<std::uint16_t>(bytes);
    } else if (sizeof(std::uint32_t) == bytes.size()) {
        convert_from_raw<std::uint32_t>(bytes);
    } else if (sizeof(std::uint64_t) == bytes.size()) {
        convert_from_raw<std::uint64_t>(bytes);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Wrong number of bytes: ")(
            bytes.size())
            .Flush();
        output = false;
    }

    return output;
}

auto CompactSize::Encode() const noexcept -> std::vector<std::byte>
{
    auto output = Space{};
    Encode(writer(output));

    return output;
}

auto CompactSize::Encode(AllocateOutput destination) const noexcept -> bool
{
    if (false == bool(destination)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid output allocator")
            .Flush();

        return false;
    }

    auto size = Size();
    const auto out = destination(size);

    if (false == out.valid(size)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to allocate output")
            .Flush();

        return false;
    }

    auto it = static_cast<std::byte*>(out.data());

    if (data_ <= OT_COMPACT_SIZE_THRESHOLD_1) {
        convert_to_raw<std::uint8_t>(preallocated(size, it));
    } else if (data_ <= OT_COMPACT_SIZE_THRESHOLD_3) {
        *it = std::byte{OT_COMPACT_SIZE_PREFIX_3};
        std::advance(it, 1);
        size -= 1;
        convert_to_raw<std::uint16_t>(preallocated(size, it));
    } else if (data_ <= OT_COMPACT_SIZE_THRESHOLD_5) {
        *it = std::byte{std::byte(OT_COMPACT_SIZE_PREFIX_5)};
        std::advance(it, 1);
        size -= 1;
        convert_to_raw<std::uint32_t>(preallocated(size, it));
    } else {
        *it = std::byte{std::byte(OT_COMPACT_SIZE_PREFIX_9)};
        std::advance(it, 1);
        size -= 1;
        convert_to_raw<std::uint64_t>(preallocated(size, it));
    }

    return true;
}

auto CompactSize::Size() const noexcept -> std::size_t
{
    if (data_ <= OT_COMPACT_SIZE_THRESHOLD_1) {
        return 1;
    } else if (data_ <= OT_COMPACT_SIZE_THRESHOLD_3) {
        return 3;
    } else if (data_ <= OT_COMPACT_SIZE_THRESHOLD_5) {
        return 5;
    } else {
        return 9;
    }
}

auto CompactSize::Total() const noexcept -> std::size_t
{
    return Size() + static_cast<std::size_t>(data_);
}

auto CompactSize::Value() const noexcept -> std::uint64_t { return data_; }
}  // namespace opentxs::blockchain::bitcoin
