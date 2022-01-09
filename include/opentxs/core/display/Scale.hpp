// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class Amount;
}  // namespace opentxs

namespace opentxs::display
{
class Scale
{
public:
    class Imp;

    /// A ratio should express the quantity of smallest values (from Amount)
    /// which represent a scale unit.
    using Ratio = std::pair<std::uint8_t, std::int8_t>;
    using OptionalInt = std::optional<std::uint8_t>;

    auto DefaultMinDecimals() const noexcept -> OptionalInt;
    auto DefaultMaxDecimals() const noexcept -> OptionalInt;
    auto Format(
        const Amount& amount,
        const OptionalInt minDecimals = std::nullopt,
        const OptionalInt maxDecimals = std::nullopt) const noexcept(false)
        -> UnallocatedCString;
    auto Import(const UnallocatedCString& formatted) const noexcept(false)
        -> Amount;
    auto MaximumDecimals() const noexcept -> std::uint8_t;
    auto Prefix() const noexcept -> UnallocatedCString;
    auto Ratios() const noexcept -> const UnallocatedVector<Ratio>&;
    auto Suffix() const noexcept -> UnallocatedCString;

    virtual auto swap(Scale& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT Scale(Imp* imp) noexcept;
    Scale() noexcept;
    Scale(
        const UnallocatedCString& prefix,
        const UnallocatedCString& suffix,
        const UnallocatedVector<Ratio>& ratios,
        const OptionalInt defaultMinDecimals = std::nullopt,
        const OptionalInt defaultMaxDecimals = std::nullopt) noexcept;
    Scale(const Scale&) noexcept;
    Scale(Scale&&) noexcept;

    virtual ~Scale();

private:
    Imp* imp_;

    auto operator=(const Scale&) -> Scale& = delete;
    auto operator=(Scale&&) -> Scale& = delete;
};
}  // namespace opentxs::display
