// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "display/Definition.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <stdexcept>

#include "display/Definition_imp.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs::display
{
Definition::Definition(Scales&& scales) noexcept
    : imp_(std::make_unique<Imp>(std::move(scales)))
{
    OT_ASSERT(imp_);
}

Definition::Definition() noexcept
    : Definition(Scales{})
{
}

Definition::Definition(const Definition& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    OT_ASSERT(imp_);
}

Definition::Definition(Definition&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    OT_ASSERT(imp_);
}

auto Definition::operator=(const Definition& rhs) noexcept -> Definition&
{
    if (&rhs != this) { imp_ = std::make_unique<Imp>(*rhs.imp_); }

    return *this;
}

auto Definition::operator=(Definition&& rhs) noexcept -> Definition&
{
    if (&rhs != this) { std::swap(imp_, rhs.imp_); }

    return *this;
}

auto Definition::Format(
    const Amount amount,
    const Index index,
    const OptionalInt minDecimals,
    const OptionalInt maxDecimals) const noexcept(false) -> std::string
{
    try {
        const auto& scale = imp_->scales_.at(static_cast<std::size_t>(index));

        return scale.second.Format(amount, minDecimals, maxDecimals);
    } catch (...) {
        throw std::out_of_range("Invalid scale index");
    }
}

auto Definition::GetScales() const noexcept -> const Map&
{
    imp_->Populate();

    return imp_->cached_.value();
}

auto Definition::Import(const std::string& formatted, const Index scale) const
    noexcept(false) -> Amount
{
    return imp_->Import(formatted, scale);
}

Definition::~Definition() = default;
}  // namespace opentxs::display
