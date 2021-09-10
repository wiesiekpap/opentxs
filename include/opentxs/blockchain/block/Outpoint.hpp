// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_OUTPOINT_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_OUTPOINT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

#include "opentxs/Bytes.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
struct OPENTXS_EXPORT Outpoint {
    std::array<std::byte, 32> txid_{};
    std::array<std::byte, 4> index_{};

    auto operator<(const Outpoint& rhs) const noexcept -> bool;
    auto operator<=(const Outpoint& rhs) const noexcept -> bool;
    auto operator>(const Outpoint& rhs) const noexcept -> bool;
    auto operator>=(const Outpoint& rhs) const noexcept -> bool;
    auto operator==(const Outpoint& rhs) const noexcept -> bool;
    auto operator!=(const Outpoint& rhs) const noexcept -> bool;

    auto Bytes() const noexcept -> ReadView;
    auto Index() const noexcept -> std::uint32_t;
    auto Txid() const noexcept -> ReadView;
    auto str() const noexcept -> std::string;

    Outpoint(const ReadView serialized) noexcept(false);
    Outpoint(const ReadView txid, const std::uint32_t index) noexcept(false);
    Outpoint() noexcept;
    Outpoint(const Outpoint&) noexcept;
    Outpoint(Outpoint&&) noexcept;
    auto operator=(const Outpoint&) noexcept -> Outpoint&;
    auto operator=(Outpoint&&) noexcept -> Outpoint&;
};
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
