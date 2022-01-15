// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <optional>

#include "opentxs/core/Types.hpp"
#include "opentxs/core/display/Scale.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class Amount;
}  // namespace opentxs

namespace opentxs::display
{
class Definition
{
public:
    using Index = unsigned int;
    using Name = UnallocatedCString;
    using NamedScale = std::pair<Name, Scale>;
    using Map = UnallocatedMap<Index, Name>;
    using Scales = UnallocatedVector<NamedScale>;
    using OptionalInt = Scale::OptionalInt;

    auto DisplayScales() const noexcept -> const Scales&;
    auto Format(
        const Amount amount,
        const Index scale = 0,
        const OptionalInt minDecimals = std::nullopt,
        const OptionalInt maxDecimals = std::nullopt) const noexcept(false)
        -> UnallocatedCString;
    auto GetScales() const noexcept -> const Map&;
    auto Import(const UnallocatedCString& formatted, const Index scale = 0)
        const noexcept(false) -> Amount;
    auto ShortName() const noexcept -> UnallocatedCString;

    virtual auto swap(Definition& rhs) noexcept -> void;

    Definition(UnallocatedCString&& shortname, Scales&& scales) noexcept;
    Definition() noexcept;
    Definition(const Definition&) noexcept;
    Definition(Definition&&) noexcept;

    auto operator=(const Definition&) noexcept -> Definition&;
    auto operator=(Definition&&) noexcept -> Definition&;

    virtual ~Definition();

private:
    struct Imp;

    Imp* imp_;
};

auto GetDefinition(UnitType) noexcept -> const Definition&;
}  // namespace opentxs::display
