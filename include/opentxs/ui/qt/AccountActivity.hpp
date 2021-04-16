// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTACTIVITYQT_HPP
#define OPENTXS_UI_ACCOUNTACTIVITYQT_HPP

#include <QIdentityProxyModel>

#include "opentxs/opentxs_export.hpp"              // IWYU pragma: keep
#include "opentxs/ui/qt/AmountValidator.hpp"       // IWYU pragma: keep
#include "opentxs/ui/qt/DestinationValidator.hpp"  // IWYU pragma: keep
#include "opentxs/ui/qt/DisplayScale.hpp"          // IWYU pragma: keep

namespace opentxs
{
namespace ui
{
namespace implementation
{
class AccountActivity;
}  // namespace implementation

class AccountActivityQt;
class DisplayScaleQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::AccountActivityQt final
    : public QIdentityProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QObject* amountValidator READ getAmountValidator CONSTANT)
    Q_PROPERTY(QObject* destValidator READ getDestValidator CONSTANT)
    Q_PROPERTY(QObject* scaleModel READ getScaleModel CONSTANT)
    Q_PROPERTY(QString accountID READ accountID NOTIFY updated)
    Q_PROPERTY(int balancePolarity READ balancePolarity NOTIFY updated)
    Q_PROPERTY(QList<int> depositChains READ depositChains NOTIFY updated)
    Q_PROPERTY(QString displayBalance READ displayBalance NOTIFY updated)
    Q_PROPERTY(
        double syncPercentage READ syncPercentage NOTIFY syncPercentageUpdated)
    using Progress = QPair<int, int>;
    Q_PROPERTY(
        Progress syncProgress READ syncProgress NOTIFY syncProgressUpdated)

signals:
    void updated() const;
    void syncPercentageUpdated(double) const;
    void syncProgressUpdated(int, int) const;

public:
    // User roles return the same data for all columns
    //
    // PolarityRole: int (-1, 0, or 1)
    // ContactsRole: QStringList
    // WorkflowRole: QString
    // TypeRole: int (opentxs::StorageBox)
    //
    // Qt::DisplayRole, AmountColumn: QString
    // Qt::DisplayRole, TextColumn: QString
    // Qt::DisplayRole, MemoColumn: QString
    // Qt::DisplayRole, TimeColumn: QDateTime
    // Qt::DisplayRole, UUIDColumn: QString

    enum Roles {
        PolarityRole = Qt::UserRole + 0,
        ContactsRole = Qt::UserRole + 1,
        WorkflowRole = Qt::UserRole + 2,
        TypeRole = Qt::UserRole + 3,
    };
    enum Columns {
        AmountColumn = 0,
        TextColumn = 1,
        MemoColumn = 2,
        TimeColumn = 3,
        UUIDColumn = 4,
    };

    QString accountID() const noexcept;
    int balancePolarity() const noexcept;
    QList<int> depositChains() const noexcept;
    QString displayBalance() const noexcept;
    AmountValidator* getAmountValidator() const noexcept;
    DestinationValidator* getDestValidator() const noexcept;
    DisplayScaleQt* getScaleModel() const noexcept;
    Q_INVOKABLE bool sendToAddress(
        const QString& address,
        const QString& amount,
        const QString& memo) const noexcept;
    Q_INVOKABLE bool sendToContact(
        const QString& contactID,
        const QString& amount,
        const QString& memo) const noexcept;
    Q_INVOKABLE QString getDepositAddress(const int chain = 0) const noexcept;
    double syncPercentage() const noexcept;
    QPair<int, int> syncProgress() const noexcept;
    Q_INVOKABLE bool validateAddress(const QString& address) const noexcept;
    Q_INVOKABLE QString validateAmount(const QString& amount) const noexcept;

    AccountActivityQt(implementation::AccountActivity& parent) noexcept;

    ~AccountActivityQt() final = default;

private:
    implementation::AccountActivity& parent_;

    void notify() const noexcept;

    void init() noexcept;

    AccountActivityQt() = delete;
    AccountActivityQt(const AccountActivityQt&) = delete;
    AccountActivityQt(AccountActivityQt&&) = delete;
    AccountActivityQt& operator=(const AccountActivityQt&) = delete;
    AccountActivityQt& operator=(AccountActivityQt&&) = delete;
};
#endif
