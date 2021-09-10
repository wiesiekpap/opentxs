// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTACTIVITYQT_HPP
#define OPENTXS_UI_ACCOUNTACTIVITYQT_HPP

#include <QObject>
#include <QString>
#include <QVariant>

#include "opentxs/opentxs_export.hpp"              // IWYU pragma: keep
#include "opentxs/ui/qt/AmountValidator.hpp"       // IWYU pragma: keep
#include "opentxs/ui/qt/DestinationValidator.hpp"  // IWYU pragma: keep
#include "opentxs/ui/qt/DisplayScale.hpp"          // IWYU pragma: keep
#include "opentxs/ui/qt/Model.hpp"

class QObject;

namespace opentxs
{
namespace ui
{
namespace internal
{
struct AccountActivity;
}  // namespace internal

class AccountActivityQt;
class DisplayScaleQt;
}  // namespace ui
}  // namespace opentxs

class OPENTXS_EXPORT opentxs::ui::AccountActivityQt final : public qt::Model
{
    Q_OBJECT
    Q_PROPERTY(QObject* amountValidator READ getAmountValidator CONSTANT)
    Q_PROPERTY(QObject* destValidator READ getDestValidator CONSTANT)
    Q_PROPERTY(QObject* scaleModel READ getScaleModel CONSTANT)
    Q_PROPERTY(QString accountID READ accountID CONSTANT)
    Q_PROPERTY(
        int balancePolarity READ balancePolarity NOTIFY balancePolarityChanged)
    Q_PROPERTY(QVariantList depositChains READ depositChains CONSTANT)
    Q_PROPERTY(QString displayBalance READ displayBalance NOTIFY balanceChanged)
    Q_PROPERTY(
        double syncPercentage READ syncPercentage NOTIFY syncPercentageUpdated)
    Q_PROPERTY(
        QVariantList syncProgress READ syncProgress NOTIFY syncProgressUpdated)

signals:
    void balanceChanged(QString) const;
    void balancePolarityChanged(int) const;
    void transactionSendResult(int, int, QString) const;
    void syncPercentageUpdated(double) const;
    void syncProgressUpdated(int, int) const;

public:
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE int sendToAddress(
        const QString& address,
        const QString& amount,
        const QString& memo,
        int scale = 0) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE int sendToContact(
        const QString& contactID,
        const QString& amount,
        const QString& memo,
        int scale = 0) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QString getDepositAddress(const int chain = 0) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE bool validateAddress(const QString& address) const noexcept;
    // NOLINTNEXTLINE(modernize-use-trailing-return-type)
    Q_INVOKABLE QString validateAmount(const QString& amount) const noexcept;

public:
    // Five columns when used in a table view
    //
    // All data is available in a list view via the user roles defined below:
    enum Roles {
        AmountRole = Qt::UserRole + 0,    // QString
        TextRole = Qt::UserRole + 1,      // QString
        MemoRole = Qt::UserRole + 2,      // QString
        TimeRole = Qt::UserRole + 3,      // QDateTime
        UUIDRole = Qt::UserRole + 4,      // QString
        PolarityRole = Qt::UserRole + 5,  // int, -1, 0, or 1
        ContactsRole = Qt::UserRole + 6,  // QStringList
        WorkflowRole = Qt::UserRole + 7,  // QString
        TypeRole = Qt::UserRole + 8,      // int, opentxs::StorageBox)
    };
    enum Columns {
        TimeColumn = 0,
        TextColumn = 1,
        AmountColumn = 2,
        UUIDColumn = 3,
        MemoColumn = 4,
    };

    QString accountID() const noexcept;
    int balancePolarity() const noexcept;
    // Each item in the list is an opentxs::blockchain::Type enum value cast to
    // an int
    QVariantList depositChains() const noexcept;
    QString displayBalance() const noexcept;
    AmountValidator* getAmountValidator() const noexcept;
    DestinationValidator* getDestValidator() const noexcept;
    DisplayScaleQt* getScaleModel() const noexcept;
    auto headerData(
        int section,
        Qt::Orientation orientation,
        int role = Qt::DisplayRole) const noexcept -> QVariant final;
    double syncPercentage() const noexcept;
    QVariantList syncProgress() const noexcept;

    AccountActivityQt(internal::AccountActivity& parent) noexcept;

    ~AccountActivityQt() final;

private:
    struct Imp;

    Imp* imp_;

    AccountActivityQt() = delete;
    AccountActivityQt(const AccountActivityQt&) = delete;
    AccountActivityQt(AccountActivityQt&&) = delete;
    AccountActivityQt& operator=(const AccountActivityQt&) = delete;
    AccountActivityQt& operator=(AccountActivityQt&&) = delete;
};
#endif
