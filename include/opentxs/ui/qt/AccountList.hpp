// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTLISTQT_HPP
#define OPENTXS_UI_ACCOUNTLISTQT_HPP

#include <QObject>
#include <QString>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep
#include "opentxs/ui/qt/Model.hpp"

class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct AccountList;
}  // namespace internal

class AccountListQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::AccountListQt final : public qt::Model
{
    Q_OBJECT

public:
    // User roles return the same data for all columns
    //
    // NotaryIDRole: QString
    // UnitRole: int (contact::ContactItemType)
    // AccountIDRole: QString
    // BalanceRole: int
    // PolarityRole: int (-1, 0, or 1)
    // AccountTypeRole: int (opentxs::AccountType)
    // ContractIdRole: QString
    //
    // Qt::DisplayRole, NotaryNameColumn: QString
    // Qt::DisplayRole, DisplayUnitColumn: QString
    // Qt::DisplayRole, AccountNameColumn: QString
    // Qt::DisplayRole, DisplayBalanceColumn: QString

    enum Columns {
        NotaryNameColumn = 0,
        DisplayUnitColumn = 1,
        AccountNameColumn = 2,
        DisplayBalanceColumn = 3,
    };
    enum Roles {
        NotaryIDRole = Qt::UserRole + 0,
        UnitRole = Qt::UserRole + 1,
        AccountIDRole = Qt::UserRole + 2,
        BalanceRole = Qt::UserRole + 3,
        PolarityRole = Qt::UserRole + 4,
        AccountTypeRole = Qt::UserRole + 5,
        ContractIdRole = Qt::UserRole + 6,
    };

    AccountListQt(internal::AccountList& parent) noexcept;

    ~AccountListQt() final;

private:
    struct Imp;

    Imp* imp_;

    AccountListQt(const AccountListQt&) = delete;
    AccountListQt(AccountListQt&&) = delete;
    AccountListQt& operator=(const AccountListQt&) = delete;
    AccountListQt& operator=(AccountListQt&&) = delete;
};
#endif
