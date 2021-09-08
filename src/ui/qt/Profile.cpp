// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "opentxs/ui/qt/Profile.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "internal/ui/UI.hpp"

namespace opentxs::factory
{
auto ProfileQtModel(ui::internal::Profile& parent) noexcept
    -> std::unique_ptr<ui::ProfileQt>
{
    using ReturnType = ui::ProfileQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ProfileQt::Imp {
    internal::Profile& parent_;

    Imp(internal::Profile& parent)
        : parent_(parent)
    {
    }
};

ProfileQt::ProfileQt(internal::Profile& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) { internal_->SetColumnCount(nullptr, 1); }

    imp_->parent_.SetCallbacks(
        {[this](std::string name) { emit displayNameChanged(name.c_str()); },
         [this](std::string paymentCode) {
             emit paymentCodeChanged(paymentCode.c_str());
         }});
}

auto ProfileQt::displayName() const noexcept -> QString
{
    return imp_->parent_.DisplayName().c_str();
}

auto ProfileQt::nymID() const noexcept -> QString
{
    return imp_->parent_.ID().c_str();
}

auto ProfileQt::paymentCode() const noexcept -> QString
{
    return imp_->parent_.PaymentCode().c_str();
}

ProfileQt::~ProfileQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui
