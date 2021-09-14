// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/accountactivity/BlockchainAccountActivity.hpp"  // IWYU pragma: associated
#include "ui/accountlist/BlockchainAccountListItem.hpp"  // IWYU pragma: associated

#include <QObject>
#include <QVariant>
#include <string>
#include <utility>

#include "display/Definition.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/qt/AccountList.hpp"
#include "ui/base/Widget.hpp"
#include "ui/qt/SendMonitor.hpp"
#include "util/Polarity.hpp"

namespace opentxs::ui::implementation
{
auto BlockchainAccountActivity::Send(
    const std::string& address,
    const std::string& input,
    const std::string& memo,
    Scale scale,
    SendMonitor::Callback cb) const noexcept -> int
{
    try {
        const auto& network =
            Widget::api_.Network().Blockchain().GetChain(chain_);
        const auto recipient = Widget::api_.Factory().PaymentCode(address);
        const auto amount = scales_.Import(input, scale);

        if (0 < recipient->Version()) {

            return SendMonitor().watch(
                network.SendToPaymentCode(primary_id_, recipient, amount, memo),
                std::move(cb));
        } else {

            return SendMonitor().watch(
                network.SendToAddress(primary_id_, address, amount, memo),
                std::move(cb));
        }
    } catch (...) {

        return -1;
    }
}

auto BlockchainAccountListItem::qt_data(
    const int column,
    const int role,
    QVariant& out) const noexcept -> void
{
    using Parent = AccountListQt;

    switch (role) {
        case Qt::DisplayRole: {
            switch (column) {
                case Parent::NotaryNameColumn: {
                    qt_data(column, Parent::NotaryNameRole, out);
                } break;
                case Parent::DisplayUnitColumn: {
                    qt_data(column, Parent::UnitNameRole, out);
                } break;
                case Parent::AccountNameColumn: {
                    qt_data(column, Parent::NameRole, out);
                } break;
                case Parent::DisplayBalanceColumn: {
                    qt_data(column, Parent::BalanceRole, out);
                } break;
                default: {
                }
            }
        } break;
        case Parent::NameRole: {
            out = Name().c_str();
        } break;
        case Parent::NotaryIDRole: {
            out = NotaryID().c_str();
        } break;
        case Parent::NotaryNameRole: {
            out = NotaryName().c_str();
        } break;
        case Parent::UnitRole: {
            out = static_cast<int>(Unit());
        } break;
        case Parent::UnitNameRole: {
            out = DisplayUnit().c_str();
        } break;
        case Parent::AccountIDRole: {
            out = AccountID().c_str();
        } break;
        case Parent::BalanceRole: {
            out = DisplayBalance().c_str();
        } break;
        case Parent::PolarityRole: {
            out = polarity(Balance());
        } break;
        case Parent::AccountTypeRole: {
            out = static_cast<int>(Type());
        } break;
        case Parent::ContractIdRole: {
            out = ContractID().c_str();
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
