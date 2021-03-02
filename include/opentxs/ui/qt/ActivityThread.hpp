// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACTIVITYTHREADQT_HPP
#define OPENTXS_UI_ACTIVITYTHREADQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"  // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class ActivityThread;
}  // namespace implementation

class ActivityThreadQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::ActivityThreadQt final
    : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString displayName READ displayName NOTIFY updated)
    Q_PROPERTY(QString draft READ getDraft NOTIFY updated)
    Q_PROPERTY(QString participants READ participants NOTIFY updated)
    Q_PROPERTY(QString threadID READ threadID NOTIFY updated)

signals:
    void updated() const;

public:
    // Table layout
    enum Roles {
        PolarityRole = Qt::UserRole + 0,
        TypeRole = Qt::UserRole + 1,
    };
    enum Columns {
        TextColumn = 0,
        AmountColumn = 1,
        MemoColumn = 2,
        TimeColumn = 3,
        LoadingColumn = 4,
        PendingColumn = 5,
    };

    QString displayName() const noexcept;
    QString getDraft() const noexcept;
    QString participants() const noexcept;
    QString threadID() const noexcept;
    Q_INVOKABLE bool pay(
        const QString& amount,
        const QString& sourceAccount,
        const QString& memo = "") const noexcept;
    Q_INVOKABLE QString paymentCode(const int currency) const noexcept;
    Q_INVOKABLE bool sendDraft() const noexcept;
    Q_INVOKABLE bool setDraft(const QString& draft) const noexcept;

    ActivityThreadQt(implementation::ActivityThread& parent) noexcept;

    ~ActivityThreadQt() final = default;

private:
    implementation::ActivityThread& parent_;

    void notify() const noexcept;

    ActivityThreadQt() = delete;
    ActivityThreadQt(const ActivityThreadQt&) = delete;
    ActivityThreadQt(ActivityThreadQt&&) = delete;
    ActivityThreadQt& operator=(const ActivityThreadQt&) = delete;
    ActivityThreadQt& operator=(ActivityThreadQt&&) = delete;
};
#endif
