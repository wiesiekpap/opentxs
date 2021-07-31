// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "opentxs/ui/qt/ActivityThread.hpp"  // IWYU pragma: associated

#include <QString>
#include <memory>
#include <string>

#include "internal/ui/UI.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "ui/activitythread/ActivityThread.hpp"
#include "ui/activitythread/DraftValidator.hpp"

class QValidator;

namespace opentxs::factory
{
auto ActivityThreadQtModel(ui::implementation::ActivityThread& parent) noexcept
    -> std::unique_ptr<ui::ActivityThreadQt>
{
    using ReturnType = ui::ActivityThreadQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct ActivityThreadQt::Imp {
    implementation::ActivityThread& parent_;
    implementation::DraftValidator validator_;

    Imp(implementation::ActivityThread& parent) noexcept
        : parent_(parent)
        , validator_(parent_)
    {
    }
};

ActivityThreadQt::ActivityThreadQt(
    implementation::ActivityThread& parent) noexcept
    : imp_(std::make_unique<Imp>(parent).release())
{
    OT_ASSERT(nullptr != imp_);

    claim_ownership(this);
    imp_->parent_.SetCallbacks({
        [this]() -> void { emit updated(); },
        [this]() -> void { emit displayNameUpdate(); },
        [this]() -> void { emit draftUpdate(); },
        [this](bool value) -> void { emit canMessageUpdate(value); },
    });
    setSourceModel(&(imp_->parent_));
}

auto ActivityThreadQt::canMessage() const noexcept -> bool
{
    return imp_->parent_.CanMessage();
}

auto ActivityThreadQt::displayName() const noexcept -> QString
{
    return imp_->parent_.DisplayName().c_str();
}

auto ActivityThreadQt::draft() const noexcept -> QString
{
    return imp_->parent_.GetDraft().c_str();
}

auto ActivityThreadQt::draftValidator() const noexcept -> QValidator*
{
    return &(imp_->validator_);
}

auto ActivityThreadQt::headerData(int section, Qt::Orientation, int role) const
    -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case TimeColumn: {
            return "Time";
        }
        case FromColumn: {
            return "Sender";
        }
        case TextColumn: {
            return "Event";
        }
        case AmountColumn: {
            return "Amount";
        }
        case MemoColumn: {
            return "Memo";
        }
        case LoadingColumn: {
            return "Loading";
        }
        case PendingColumn: {
            return "Pending";
        }
        default: {

            return {};
        }
    }
}

auto ActivityThreadQt::participants() const noexcept -> QString
{
    return imp_->parent_.Participants().c_str();
}

auto ActivityThreadQt::pay(
    const QString& amount,
    const QString& sourceAccount,
    const QString& memo) const noexcept -> bool
{
    return imp_->parent_.Pay(
        amount.toStdString(),
        Identifier::Factory(sourceAccount.toStdString()),
        memo.toStdString(),
        PaymentType::Cheque);
}

auto ActivityThreadQt::paymentCode(const int currency) const noexcept -> QString
{
    return imp_->parent_
        .PaymentCode(static_cast<contact::ContactItemType>(currency))
        .c_str();
}

auto ActivityThreadQt::sendDraft() const noexcept -> bool
{
    return imp_->parent_.SendDraft();
}

auto ActivityThreadQt::setDraft(QString draft) -> void
{
    imp_->parent_.SetDraft(draft.toStdString());
}

auto ActivityThreadQt::threadID() const noexcept -> QString
{
    return imp_->parent_.ThreadID().c_str();
}

ActivityThreadQt::~ActivityThreadQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui
