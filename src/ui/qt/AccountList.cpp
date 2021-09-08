// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                   // IWYU pragma: associated
#include "1_Internal.hpp"                 // IWYU pragma: associated
#include "opentxs/ui/qt/AccountList.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <memory>
#include <string>

#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "ui/accountlist/AccountListItem.hpp"
#include "util/Polarity.hpp"  // IWYU pragma: keep

namespace opentxs::factory
{
auto AccountListQtModel(ui::internal::AccountList& parent) noexcept
    -> std::unique_ptr<ui::AccountListQt>
{
    using ReturnType = ui::AccountListQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct AccountListQt::Imp {
    internal::AccountList& parent_;

    Imp(internal::AccountList& parent)
        : parent_(parent)
    {
    }
};

AccountListQt::AccountListQt(internal::AccountList& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 4);
        internal_->SetRoleData({
            {AccountListQt::NotaryIDRole, "notary"},
            {AccountListQt::UnitRole, "unit"},
            {AccountListQt::AccountIDRole, "account"},
            {AccountListQt::BalanceRole, "balance"},
            {AccountListQt::PolarityRole, "polarity"},
            {AccountListQt::AccountTypeRole, "accounttype"},
            {AccountListQt::ContractIdRole, "contractid"},
        });
    }
}

AccountListQt::~AccountListQt()
{
    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
auto AccountListItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    switch (role) {
        case AccountListQt::NotaryIDRole: {
            out = NotaryID().c_str();
        } break;
        case AccountListQt::UnitRole: {
            out = static_cast<int>(Unit());
        } break;
        case AccountListQt::AccountIDRole: {
            out = AccountID().c_str();
        } break;
        case AccountListQt::BalanceRole: {
            out = static_cast<unsigned long long>(Balance());
        } break;
        case AccountListQt::PolarityRole: {
            out = polarity(Balance());
        } break;
        case AccountListQt::AccountTypeRole: {
            out = static_cast<int>(Type());
        } break;
        case AccountListQt::ContractIdRole: {
            out = ContractID().c_str();
        } break;
        case Qt::DisplayRole: {
            switch (column) {
                case AccountListQt::NotaryNameColumn: {
                    out = NotaryName().c_str();
                } break;
                case AccountListQt::DisplayUnitColumn: {
                    out = DisplayUnit().c_str();
                } break;
                case AccountListQt::AccountNameColumn: {
                    out = Name().c_str();
                } break;
                case AccountListQt::DisplayBalanceColumn: {
                    out = DisplayBalance().c_str();
                } break;
                default: {
                }
            }
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
