// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "opentxs/Bytes.hpp"

namespace opentxs::blockchain::bitcoin
{
class CompactSize
{
public:
    using Bytes = std::vector<std::byte>;

    // Returns the number of bytes SUBSEQUENT to first_byte
    // Possible output values are: 0, 2, 4, 8
    OPENTXS_EXPORT static auto CalculateSize(const std::byte first) noexcept
        -> std::uint64_t;

    OPENTXS_EXPORT auto Encode() const noexcept -> Bytes;
    OPENTXS_EXPORT auto Encode(AllocateOutput destination) const noexcept
        -> bool;
    // Number of bytes the CompactSize will occupy
    OPENTXS_EXPORT auto Size() const noexcept -> std::size_t;
    // Number of bytes the CompactSize and associated data will occupy
    OPENTXS_EXPORT auto Total() const noexcept -> std::size_t;
    OPENTXS_EXPORT auto Value() const noexcept -> std::uint64_t;

    // Initial marker byte should be omitted
    // Valid inputs are 0, 2, 4, or 8 bytes
    OPENTXS_EXPORT auto Decode(const Bytes& bytes) noexcept -> bool;

    OPENTXS_EXPORT CompactSize() noexcept = default;
    OPENTXS_EXPORT explicit CompactSize(std::uint64_t value) noexcept;
    // Initial marker byte should be omitted
    // Valid inputs are 1, 2, 4, or 8 bytes
    // Throws std::invalid_argument for invalid input
    OPENTXS_EXPORT CompactSize(const Bytes& bytes) noexcept(false);
    OPENTXS_EXPORT CompactSize(const CompactSize&) noexcept = default;
    OPENTXS_EXPORT CompactSize(CompactSize&&) noexcept = default;
    OPENTXS_EXPORT auto operator=(const CompactSize&) noexcept
        -> CompactSize& = default;
    OPENTXS_EXPORT auto operator=(CompactSize&&) noexcept
        -> CompactSize& = default;
    OPENTXS_EXPORT auto operator=(const std::uint64_t rhs) noexcept
        -> CompactSize&;

    OPENTXS_EXPORT ~CompactSize() = default;

private:
    std::uint64_t data_;

    template <typename SizeType>
    auto convert_to_raw(AllocateOutput output) const noexcept -> bool;
    template <typename SizeType>
    auto convert_from_raw(const Bytes& bytes) noexcept -> void;
};
}  // namespace opentxs::blockchain::bitcoin
