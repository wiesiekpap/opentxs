// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "opentxs/ui/qt/AccountActivity.hpp"  // IWYU pragma: associated

#include <QList>
#include <QObject>
#include <QString>
#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "ui/accountactivity/AccountActivity.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs::factory
{
auto AccountActivityQtModel(
    ui::implementation::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>
{
    using ReturnType = ui::AccountActivityQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER_EXTRA(AccountActivityQt, implementation::AccountActivity)

auto AccountActivityQt::accountID() const noexcept -> QString
{
    return parent_.AccountID().c_str();
}

auto AccountActivityQt::balancePolarity() const noexcept -> int
{
    return parent_.BalancePolarity();
}

auto AccountActivityQt::depositChains() const noexcept -> QVariantList
{
    const auto input = parent_.DepositChains();
    auto output = QVariantList{};
    std::transform(
        std::begin(input), std::end(input), std::back_inserter(output), [
        ](const auto& in) -> auto { return static_cast<int>(in); });

    return output;
}

auto AccountActivityQt::displayBalance() const noexcept -> QString
{
    return parent_.DisplayBalance().c_str();
}

auto AccountActivityQt::getAmountValidator() const noexcept -> AmountValidator*
{
    return &parent_.amount_validator_;
}

auto AccountActivityQt::getDepositAddress(const int chain) const noexcept
    -> QString
{
    return parent_.DepositAddress(static_cast<blockchain::Type>(chain)).c_str();
}

auto AccountActivityQt::getDestValidator() const noexcept
    -> DestinationValidator*
{
    return &parent_.destination_validator_;
}

auto AccountActivityQt::getScaleModel() const noexcept -> DisplayScaleQt*
{
    return &parent_.scales_qt_;
}

auto AccountActivityQt::headerData(int section, Qt::Orientation, int role) const
    -> QVariant
{
    if (Qt::DisplayRole != role) { return {}; }

    switch (section) {
        case TimeColumn: {
            return "Time";
        }
        case TextColumn: {
            return "Event";
        }
        case AmountColumn: {
            return "Amount";
        }
        case UUIDColumn: {
            return "Transaction ID";
        }
        case MemoColumn: {
            return "Memo";
        }
        default: {

            return {};
        }
    }
}

auto AccountActivityQt::init() noexcept -> void
{
    parent_.SetSyncCallback([this](int current, int max, double percent) {
        emit syncPercentageUpdated(percent);
        emit syncProgressUpdated(current, max);
    });
}

auto AccountActivityQt::sendToAddress(
    const QString& address,
    const QString& amount,
    const QString& memo) const noexcept -> bool
{
    return parent_.Send(
        address.toStdString(), amount.toStdString(), memo.toStdString());
}

auto AccountActivityQt::sendToContact(
    const QString& contactID,
    const QString& amount,
    const QString& memo) const noexcept -> bool
{
    return parent_.Send(
        parent_.Widget::api_.Factory().Identifier(contactID.toStdString()),
        amount.toStdString(),
        memo.toStdString());
}

auto AccountActivityQt::syncPercentage() const noexcept -> double
{
    return parent_.SyncPercentage();
}

auto AccountActivityQt::syncProgress() const noexcept -> QVariantList
{
    const auto progress = parent_.SyncProgress();
    auto out = QVariantList{};
    out.push_back(progress.first);
    out.push_back(progress.second);

    return out;
}

auto AccountActivityQt::validateAddress(const QString& address) const noexcept
    -> bool
{
    return parent_.ValidateAddress(address.toStdString());
}

auto AccountActivityQt::validateAmount(const QString& amount) const noexcept
    -> QString
{
    return parent_.ValidateAmount(amount.toStdString()).c_str();
}
}  // namespace opentxs::ui
