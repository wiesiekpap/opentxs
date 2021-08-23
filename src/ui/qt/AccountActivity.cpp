// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "opentxs/ui/qt/AccountActivity.hpp"   // IWYU pragma: associated
#include "ui/accountactivity/BalanceItem.hpp"  // IWYU pragma: associated

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <algorithm>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "core/Worker.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/qt/AmountValidator.hpp"
#include "opentxs/ui/qt/DestinationValidator.hpp"
#include "opentxs/ui/qt/DisplayScale.hpp"
#include "ui/accountactivity/AccountActivity.hpp"
#include "ui/qt/SendMonitor.hpp"
#include "util/Polarity.hpp"

namespace opentxs::factory
{
auto AccountActivityQtModel(ui::internal::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>
{
    using ReturnType = ui::AccountActivityQt;

    return std::make_unique<ReturnType>(parent);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
struct AccountActivityQt::Imp {
    internal::AccountActivity& parent_;

    Imp(internal::AccountActivity& parent)
        : parent_(parent)
    {
    }
};

AccountActivityQt::AccountActivityQt(internal::AccountActivity& parent) noexcept
    : Model(parent.GetQt())
    , imp_(std::make_unique<Imp>(parent).release())
{
    if (nullptr != internal_) {
        internal_->SetColumnCount(nullptr, 5);
        internal_->SetRoleData({
            {AccountActivityQt::AmountRole, "amount"},
            {AccountActivityQt::TextRole, "description"},
            {AccountActivityQt::MemoRole, "memo"},
            {AccountActivityQt::TimeRole, "timestamp"},
            {AccountActivityQt::UUIDRole, "uuid"},
            {AccountActivityQt::PolarityRole, "polarity"},
            {AccountActivityQt::ContactsRole, "contacts"},
            {AccountActivityQt::WorkflowRole, "workflow"},
            {AccountActivityQt::TypeRole, "type"},
        });
    }

    imp_->parent_.SetCallbacks(
        {[this](int current, int max, double percent) {
             emit syncPercentageUpdated(percent);
             emit syncProgressUpdated(current, max);
         },
         [this](std::string balance) { emit balanceChanged(balance.c_str()); },
         [this](int polarity) { emit balancePolarityChanged(polarity); }});
}

auto AccountActivityQt::accountID() const noexcept -> QString
{
    return imp_->parent_.AccountID().c_str();
}

auto AccountActivityQt::balancePolarity() const noexcept -> int
{
    return imp_->parent_.BalancePolarity();
}

auto AccountActivityQt::depositChains() const noexcept -> QVariantList
{
    const auto input = imp_->parent_.DepositChains();
    auto output = QVariantList{};
    std::transform(
        std::begin(input), std::end(input), std::back_inserter(output), [
        ](const auto& in) -> auto { return static_cast<int>(in); });

    return output;
}

auto AccountActivityQt::displayBalance() const noexcept -> QString
{
    return imp_->parent_.DisplayBalance().c_str();
}

auto AccountActivityQt::getAmountValidator() const noexcept -> AmountValidator*
{
    return &imp_->parent_.AmountValidator();
}

auto AccountActivityQt::getDepositAddress(const int chain) const noexcept
    -> QString
{
    return imp_->parent_.DepositAddress(static_cast<blockchain::Type>(chain))
        .c_str();
}

auto AccountActivityQt::getDestValidator() const noexcept
    -> DestinationValidator*
{
    return &imp_->parent_.DestinationValidator();
}

auto AccountActivityQt::getScaleModel() const noexcept -> DisplayScaleQt*
{
    return &imp_->parent_.DisplayScaleQt();
}

auto AccountActivityQt::headerData(int section, Qt::Orientation, int role)
    const noexcept -> QVariant
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

auto AccountActivityQt::sendToAddress(
    const QString& address,
    const QString& amount,
    const QString& memo,
    int scale) const noexcept -> int
{
    if (0 > scale) { return false; }

    return imp_->parent_.Send(
        address.toStdString(),
        amount.toStdString(),
        memo.toStdString(),
        static_cast<AccountActivity::Scale>(scale),
        [this](auto key, auto code, auto text) {
            emit transactionSendResult(key, code, text);
        });
}

auto AccountActivityQt::sendToContact(
    const QString& contactID,
    const QString& amount,
    const QString& memo,
    int scale) const noexcept -> int
{
    if (0 > scale) { return -1; }

    try {
        return imp_->parent_.Send(
            imp_->parent_.API().Factory().Identifier(contactID.toStdString()),
            amount.toStdString(),
            memo.toStdString(),
            static_cast<AccountActivity::Scale>(scale),
            [this](auto key, auto code, auto text) {
                emit transactionSendResult(key, code, text);
            });
    } catch (...) {

        return -1;
    }
}

auto AccountActivityQt::syncPercentage() const noexcept -> double
{
    return imp_->parent_.SyncPercentage();
}

auto AccountActivityQt::syncProgress() const noexcept -> QVariantList
{
    const auto progress = imp_->parent_.SyncProgress();
    auto out = QVariantList{};
    out.push_back(progress.first);
    out.push_back(progress.second);

    return out;
}

auto AccountActivityQt::validateAddress(const QString& address) const noexcept
    -> bool
{
    return imp_->parent_.ValidateAddress(address.toStdString());
}

auto AccountActivityQt::validateAmount(const QString& amount) const noexcept
    -> QString
{
    return imp_->parent_.ValidateAmount(amount.toStdString()).c_str();
}

AccountActivityQt::~AccountActivityQt()
{
    imp_->parent_.SendMonitor().shutdown();

    if (nullptr != imp_) {
        delete imp_;
        imp_ = nullptr;
    }
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
struct AccountActivity::QT {
    ui::DisplayScaleQt scales_qt_;
    ui::AmountValidator amount_validator_;
    ui::DestinationValidator destination_validator_;
    mutable implementation::SendMonitor send_monitor_;

    QT(AccountActivity& parent)
    noexcept
        : scales_qt_(parent.scales_)
        , amount_validator_(parent)
        , destination_validator_(
              parent.Worker::api_,
              static_cast<std::int8_t>(parent.type_),
              parent.account_id_,
              parent)
        , send_monitor_()
    {
    }
};

auto AccountActivity::DisplayScaleQt() noexcept -> ui::DisplayScaleQt&
{
    return qt_->scales_qt_;
}

auto AccountActivity::AmountValidator() noexcept -> ui::AmountValidator&
{
    return qt_->amount_validator_;
}

auto AccountActivity::DestinationValidator() noexcept
    -> ui::DestinationValidator&
{
    return qt_->destination_validator_;
}

auto AccountActivity::init_qt() noexcept -> void
{
    qt_ = std::make_unique<QT>(*this).release();

    OT_ASSERT(qt_);
}

auto AccountActivity::SendMonitor() const noexcept
    -> implementation::SendMonitor&
{
    return qt_->send_monitor_;
}

auto AccountActivity::SendMonitor() noexcept -> implementation::SendMonitor&
{
    return qt_->send_monitor_;
}

auto AccountActivity::shutdown_qt() noexcept -> void
{
    if (nullptr != qt_) {
        SendMonitor().shutdown();
        delete qt_;
        qt_ = nullptr;
    }
}

auto BalanceItem::qt_data(const int column, const int role, QVariant& out)
    const noexcept -> void
{
    switch (role) {
        case Qt::TextAlignmentRole: {
            switch (column) {
                case AccountActivityQt::TextColumn:
                case AccountActivityQt::MemoColumn: {
                    out = Qt::AlignLeft;
                } break;
                default: {
                    out = Qt::AlignHCenter;
                } break;
            }
        } break;
        case Qt::DisplayRole: {
            switch (column) {
                case AccountActivityQt::AmountColumn: {
                    qt_data(column, AccountActivityQt::AmountRole, out);
                } break;
                case AccountActivityQt::TextColumn: {
                    qt_data(column, AccountActivityQt::TextRole, out);
                } break;
                case AccountActivityQt::MemoColumn: {
                    qt_data(column, AccountActivityQt::MemoRole, out);
                } break;
                case AccountActivityQt::TimeColumn: {
                    qt_data(column, AccountActivityQt::TimeRole, out);
                } break;
                case AccountActivityQt::UUIDColumn: {
                    qt_data(column, AccountActivityQt::UUIDRole, out);
                } break;
                default: {
                }
            }
        } break;
        case AccountActivityQt::AmountRole: {
            out = DisplayAmount().c_str();
        } break;
        case AccountActivityQt::TextRole: {
            out = Text().c_str();
        } break;
        case AccountActivityQt::MemoRole: {
            out = Memo().c_str();
        } break;
        case AccountActivityQt::TimeRole: {
            auto qdatetime = QDateTime{};
            qdatetime.setSecsSinceEpoch(Clock::to_time_t(Timestamp()));

            out = qdatetime;
        } break;
        case AccountActivityQt::UUIDRole: {
            out = UUID().c_str();
        } break;
        case AccountActivityQt::PolarityRole: {
            out = polarity(Amount());
        } break;
        case AccountActivityQt::ContactsRole: {
            auto output = QStringList{};

            for (const auto& contact : Contacts()) {
                output << contact.c_str();
            }

            out = output;
        } break;
        case AccountActivityQt::WorkflowRole: {
            out = Workflow().c_str();
        } break;
        case AccountActivityQt::TypeRole: {
            out = static_cast<int>(Type());
        } break;
        default: {
        }
    };
}
}  // namespace opentxs::ui::implementation
