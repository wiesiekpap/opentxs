// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTSUMMARYQT_HPP
#define OPENTXS_UI_ACCOUNTSUMMARYQT_HPP

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
struct AccountSummary;
}  // namespace internal

class AccountSummaryQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::AccountSummaryQt final : public qt::Model
{
    Q_OBJECT

public:
    // Tree layout
    enum Roles {
        NotaryIDRole = Qt::UserRole + 0,
        AccountIDRole = Qt::UserRole + 1,
        BalanceRole = Qt::UserRole + 2,
    };
    enum Columns {
        IssuerNameColumn = 0,
        ConnectionStateColumn = 1,
        TrustedColumn = 2,
        AccountNameColumn = 3,
        BalanceColumn = 4,
    };

    AccountSummaryQt(internal::AccountSummary& parent) noexcept;

    ~AccountSummaryQt() final;

private:
    struct Imp;

    Imp* imp_;

    AccountSummaryQt(const AccountSummaryQt&) = delete;
    AccountSummaryQt(AccountSummaryQt&&) = delete;
    AccountSummaryQt& operator=(const AccountSummaryQt&) = delete;
    AccountSummaryQt& operator=(AccountSummaryQt&&) = delete;
};
#endif
