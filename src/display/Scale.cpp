// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"       // IWYU pragma: associated
#include "1_Internal.hpp"     // IWYU pragma: associated
#include "display/Scale.hpp"  // IWYU pragma: associated

#include "display/Scale_imp.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs::display
{
Scale::Scale(
    const std::string& prefix,
    const std::string& suffix,
    const std::vector<Ratio>& ratios,
    const OptionalInt defaultMinDecimals,
    const OptionalInt defaultMaxDecimals) noexcept
    : imp_(std::make_unique<Imp>(
          prefix,
          suffix,
          ratios,
          defaultMinDecimals,
          defaultMaxDecimals))
{
    OT_ASSERT(imp_);
}

Scale::Scale(const Scale& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    OT_ASSERT(imp_);
}

Scale::Scale(Scale&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Scale::Format(
    const Amount amount,
    const OptionalInt minDecimals,
    const OptionalInt maxDecimals) const noexcept(false) -> std::string
{
    return imp_->format(amount, minDecimals, maxDecimals);
}

auto Scale::Import(const std::string& formatted) const noexcept(false) -> Amount
{
    return imp_->Import(formatted);
}

auto Scale::Prefix() const noexcept -> std::string { return imp_->prefix_; }

auto Scale::Suffix() const noexcept -> std::string { return imp_->suffix_; }

Scale::~Scale() = default;
}  // namespace opentxs::display
