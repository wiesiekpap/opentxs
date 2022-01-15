// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QString>

#include "opentxs/interface/qt/Model.hpp"

class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct AccountTree;
}  // namespace internal

class AccountTreeQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::AccountTreeQt final : public qt::Model
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 0,         // QString
        NotaryIDRole = Qt::UserRole + 1,     // QString
        NotaryNameRole = Qt::UserRole + 2,   // QString
        UnitRole = Qt::UserRole + 3,         // int (UnitType)
        UnitNameRole = Qt::UserRole + 4,     // QString
        AccountIDRole = Qt::UserRole + 5,    // QString
        BalanceRole = Qt::UserRole + 6,      // QString
        PolarityRole = Qt::UserRole + 7,     // int (-1, 0, or 1)
        AccountTypeRole = Qt::UserRole + 8,  // int (opentxs::AccountType)
        ContractIdRole = Qt::UserRole + 9,   // QString
    };
    enum Columns {
        NameColumn = 0,
    };

    AccountTreeQt(internal::AccountTree& parent) noexcept;

    ~AccountTreeQt() final;

private:
    struct Imp;

    Imp* imp_;

    AccountTreeQt(const AccountTreeQt&) = delete;
    AccountTreeQt(AccountTreeQt&&) = delete;
    AccountTreeQt& operator=(const AccountTreeQt&) = delete;
    AccountTreeQt& operator=(AccountTreeQt&&) = delete;
};
