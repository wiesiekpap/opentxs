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
    switch (role) {
        case AccountListQt::NotaryIDRole: {
            out = NotaryID().c_str();
        } break;
        case AccountListQt::UnitRole: {
            out = static_cast<int>(Unit());
        } break;
        case AccountListQt::AccountIDRole: {
            out = AccountID().c_str();
        } break;
        case AccountListQt::BalanceRole: {
            out = static_cast<unsigned long long>(Balance());
        } break;
        case AccountListQt::PolarityRole: {
            out = polarity(Balance());
        } break;
        case AccountListQt::AccountTypeRole: {
            out = static_cast<int>(Type());
        } break;
        case AccountListQt::ContractIdRole: {
            out = ContractID().c_str();
        } break;
        case Qt::DisplayRole: {
            switch (column) {
                case AccountListQt::NotaryNameColumn: {
                    out = NotaryName().c_str();
                } break;
                case AccountListQt::DisplayUnitColumn: {
                    out = DisplayUnit().c_str();
                } break;
                case AccountListQt::AccountNameColumn: {
                    out = Name().c_str();
                } break;
                case AccountListQt::DisplayBalanceColumn: {
                    out = DisplayBalance().c_str();
                } break;
                default: {
                }
            }
        } break;
        default: {
        }
    }
}
}  // namespace opentxs::ui::implementation
