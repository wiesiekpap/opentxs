// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <QString>
#include <QValidator>
#include <algorithm>
#include <atomic>
#include <optional>
#include <stdexcept>
#include <string>

#include "1_Internal.hpp"
#include "display/Definition.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/qt/AmountValidator.hpp"
#include "ui/accountactivity/AccountActivity.hpp"

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

#define OT_METHOD "opentxs::ui::AmountValidator::"

namespace opentxs::ui
{
struct AmountValidator::Imp {
    using Parent = implementation::AccountActivity;
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
            LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

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
        const auto old = max_.exchange(min);

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
            fix(input, static_cast<Index>(scale_.load()));

            return State::Acceptable;
        } catch (const std::exception& e) {
            LogTrace(OT_METHOD)(__func__)(": ")(e.what()).Flush();

            return State::Invalid;
        }
    }

    Imp(Parent& parent) noexcept
        : scale_(0)
        , data_(parent.scales_)
        , min_(-1)
        , max_(-1)
    {
    }

    ~Imp() = default;

private:
    const display::Definition& data_;
    std::atomic_int min_;
    std::atomic_int max_;

    auto fix(const QString& input, Index oldScale) const noexcept(false)
        -> std::string
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

        const auto newScale = scale_.load();
        const auto min = get(min_);
        const auto max = get(max_);
        const auto data = input.toStdString();
        const auto amount = data_.Import(data, oldScale);

        return data_.Format(amount, newScale, min, max);
    }
};
}  // namespace opentxs::ui
