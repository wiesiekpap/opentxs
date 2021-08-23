// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "opentxs/ui/qt/Contact.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "internal/ui/UI.hpp"

namespace opentxs::factory
{
auto ContactQtModel(ui::internal::Contact& parent) noexcept
    -> std::unique_ptr<ui::ContactQt>
{
    using ReturnType = ui::ContactQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ContactQt::Imp {
    internal::Contact& parent_;

    Imp(internal::Contact& parent)
        : parent_(parent)
    {
    }
};

ContactQt::ContactQt(internal::Contact& parent) noexcept
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

auto ContactQt::displayName() const noexcept -> QString
{
    return imp_->parent_.DisplayName().c_str();
}

auto ContactQt::contactID() const noexcept -> QString
{
    return imp_->parent_.ContactID().c_str();
}

auto ContactQt::paymentCode() const noexcept -> QString
{
    return imp_->parent_.PaymentCode().c_str();
}

ContactQt::~ContactQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui
