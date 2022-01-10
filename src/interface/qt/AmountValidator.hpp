// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#pragma once

#include <QString>
#include <QValidator>
#include <algorithm>
#include <atomic>
#include <optional>
#include <stdexcept>

#include "1_Internal.hpp"
#include "interface/ui/accountactivity/AccountActivity.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/interface/qt/AmountValidator.hpp"
#include "opentxs/interface/ui/AccountActivity.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
namespace implementation
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation
}  // namespace implementation
}  // namespace opentxs

namespace opentxs::ui
{
struct AmountValidator::Imp {
    using Parent = ui::AccountActivity;
    using Index = display::Definition::Index;

    std::atomic<Index> scale_;

    auto fixup(QString& input) const -> void {}
    auto getMaxDecimals() const -> int { return max_; }
    auto getMinDecimals() const -> int { return min_; }
    auto getScale() const -> int { return static_cast<int>(scale_.load()); }
    auto revise(const QString& input, int previous) const -> QString
    {
        try {
            return fix(input, static_cast<Index>(std::max(previous, 0)))
                .c_str();
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }
    auto setMaxDecimals(int max) -> bool
    {
        const auto old = max_.exchange(max);

        return old != max;
    }
    auto setMinDecimals(int min) -> bool
    {
        const auto old = min_.exchange(min);

        return old != min;
    }
    auto setScale(int scale, int& old) -> bool
    {
        if (0 > scale) { return false; }

        const auto value = static_cast<Index>(scale);
        old = scale_.exchange(value);

        return static_cast<Index>(old) != value;
    }
    auto validate(QString& input, int&) const -> State
    {
        try {
            const auto fixed = fix(input, static_cast<Index>(scale_.load()));
            input = fixed.c_str();

            return State::Acceptable;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return State::Invalid;
        }
    }

    Imp(Parent& parent) noexcept
        : scale_(0)
        , parent_(parent)
        , unittype_(std::nullopt)
        , min_(-1)
        , max_(-1)
    {
    }

    ~Imp() = default;

private:
    const Parent& parent_;
    mutable std::optional<UnitType> unittype_;
    std::atomic_int min_;
    std::atomic_int max_;

    auto fix(const QString& input, Index oldScale) const noexcept(false)
        -> UnallocatedCString
    {
        static const auto get =
            [](const auto& val) -> display::Definition::OptionalInt {
            auto out = val.load();

            if (0 > out) {

                return std::nullopt;
            } else {
                return out;
            }
        };

        const auto& definition = display::GetDefinition(unittype());
        const auto newScale = scale_.load();
        const auto min = get(min_);
        const auto max = get(max_);
        const auto data = input.toStdString();
        const auto amount = definition.Import(data, oldScale);

        return definition.Format(amount, newScale, min, max);
    }

    auto unittype() const noexcept -> UnitType;
};
}  // namespace opentxs::ui
