// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "ui/accountactivity/AccountActivity.hpp"  // IWYU pragma: associated

#if OT_QT
#include <QList>
#include <QObject>
#include <QString>
#endif  // OT_QT
#include <algorithm>
#include <future>
#include <iterator>
#include <memory>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#if OT_QT
#include "opentxs/ui/qt/AccountActivity.hpp"
#endif  // OT_QT
#include "util/Work.hpp"

// #define OT_METHOD "opentxs::ui::implementation::AccountActivity::"

namespace opentxs::factory
{
#if OT_QT
auto AccountActivityQtModel(
    ui::implementation::AccountActivity& parent) noexcept
    -> std::unique_ptr<ui::AccountActivityQt>
{
    using ReturnType = ui::AccountActivityQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
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
#if OT_BLOCKCHAIN
auto AccountActivityQt::depositChains() const noexcept -> QList<int>
{
    const auto input = parent_.DepositChains();
    auto output = QList<int>{};
    std::transform(
        std::begin(input), std::end(input), std::back_inserter(output), [
        ](const auto& in) -> auto { return static_cast<int>(in); });

    return output;
}
#endif  // OT_BLOCKCHAIN
auto AccountActivityQt::displayBalance() const noexcept -> QString
{
    return parent_.DisplayBalance().c_str();
}
auto AccountActivityQt::getAmountValidator() const noexcept -> AmountValidator*
{
    return &parent_.amount_validator_;
}
#if OT_BLOCKCHAIN
auto AccountActivityQt::getDepositAddress(const int chain) const noexcept
    -> QString
{
    return parent_.DepositAddress(static_cast<blockchain::Type>(chain)).c_str();
}
#endif  // OT_BLOCKCHAIN
auto AccountActivityQt::getDestValidator() const noexcept
    -> DestinationValidator*
{
    return &parent_.destination_validator_;
}
auto AccountActivityQt::getScaleModel() const noexcept -> DisplayScaleQt*
{
    return &parent_.scales_qt_;
}
auto AccountActivityQt::init() noexcept -> void
{
#if OT_BLOCKCHAIN
    parent_.SetSyncCallback([this](int current, int max, double percent) {
        emit syncPercentageUpdated(percent);
        emit syncProgressUpdated(current, max);
    });
#endif  // OT_BLOCKCHAIN
}
#if OT_BLOCKCHAIN
auto AccountActivityQt::sendToAddress(
    const QString& address,
    const QString& amount,
    const QString& memo) const noexcept -> bool
{
    return parent_.Send(
        address.toStdString(), amount.toStdString(), memo.toStdString());
}
#endif  // OT_BLOCKCHAIN
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
#if OT_BLOCKCHAIN
auto AccountActivityQt::syncPercentage() const noexcept -> double
{
    return parent_.SyncPercentage();
}
auto AccountActivityQt::validateAddress(const QString& address) const noexcept
    -> bool
{
    return parent_.ValidateAddress(address.toStdString());
}
#endif  // OT_BLOCKCHAIN
auto AccountActivityQt::validateAmount(const QString& amount) const noexcept
    -> QString
{
    return parent_.ValidateAmount(amount.toStdString()).c_str();
}
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
AccountActivity::AccountActivity(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const AccountType type,
    const SimpleCallback& cb,
    display::Definition&& scales) noexcept
    : AccountActivityList(
          api,
          nymID,
          cb,
          true
#if OT_QT
          ,
          Roles{
              {AccountActivityQt::PolarityRole, "polarity"},
              {AccountActivityQt::ContactsRole, "contacts"},
              {AccountActivityQt::WorkflowRole, "workflow"},
              {AccountActivityQt::TypeRole, "type"},
          },
          5
#endif  // OT_QT
          )
    , Worker(api, {})
    , scales_(std::move(scales))
#if OT_QT
    , scales_qt_(scales_)
    , amount_validator_(*this)
    , destination_validator_(
          api,
          static_cast<std::int8_t>(type),
          static_cast<std::uint32_t>(Chain(api, accountID)),
          *this)
#endif  // OT_QT
    , balance_(0)
    , account_id_(accountID)
    , type_(type)
    , contract_([&] {
        try {

            return api.Wallet().UnitDefinition(
                api.Storage().AccountContract(account_id_));
        } catch (...) {

            return api.Factory().UnitDefinition();
        }
    }())
    , notary_([&] {
        try {

            return api.Wallet().Server(
                api.Storage().AccountServer(account_id_));
        } catch (...) {

            return api.Factory().ServerContract();
        }
    }())
{
}

auto AccountActivity::construct_row(
    const AccountActivityRowID& id,
    const AccountActivitySortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BalanceItem(
        *this,
        Widget::Widget::api_,
        id,
        index,
        custom,
        primary_id_,
        account_id_);
}

auto AccountActivity::init(Endpoints endpoints) noexcept -> void
{
    AccountActivityList::init();
    init_executor(std::move(endpoints));
    pipeline_->Push(MakeWork(OT_ZMQ_INIT_SIGNAL));
}

AccountActivity::~AccountActivity()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
