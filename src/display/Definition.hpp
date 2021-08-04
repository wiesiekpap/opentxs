// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "display/Scale.hpp"
#include "opentxs/Types.hpp"

namespace opentxs::display
{
class Definition
{
public:
    using Index = unsigned int;
    using Name = std::string;
    using NamedScale = std::pair<Name, Scale>;
    using Map = std::map<Index, Name>;
    using Scales = std::vector<NamedScale>;
    using OptionalInt = Scale::OptionalInt;

    auto Format(
        const Amount amount,
        const Index scale = 0,
        const OptionalInt minDecimals = std::nullopt,
        const OptionalInt maxDecimals = std::nullopt) const noexcept(false)
        -> std::string;
    auto GetScales() const noexcept -> const Map&;
    auto Import(const std::string& formatted, const Index scale = 0) const
        noexcept(false) -> Amount;

    Definition(Scales&& scales) noexcept;
    Definition() noexcept;
    Definition(const Definition&) noexcept;
    Definition(Definition&&) noexcept;

    auto operator=(const Definition&) noexcept -> Definition&;
    auto operator=(Definition&&) noexcept -> Definition&;

    ~Definition();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::display
