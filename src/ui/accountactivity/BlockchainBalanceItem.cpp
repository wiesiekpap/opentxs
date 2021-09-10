// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/accountactivity/BlockchainBalanceItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/PaymentEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflow.pb.h"
#include "ui/accountactivity/BalanceItem.hpp"
#include "ui/base/Widget.hpp"

// #define OT_METHOD "opentxs::ui::implementation::BlockchainBalanceItem::"

namespace opentxs::ui::implementation
{
BlockchainBalanceItem::BlockchainBalanceItem(
    const AccountActivityInternalInterface& parent,
    const api::client::Manager& api,
    const AccountActivityRowID& rowID,
    const AccountActivitySortKey& sortKey,
    CustomData& custom,
    const identifier::Nym& nymID,
    const Identifier& accountID,
    [[maybe_unused]] const blockchain::Type chain,
    const OTData txid,
    const opentxs::Amount amount,
    const std::string memo,
    const std::string text) noexcept
    : BalanceItem(parent, api, rowID, sortKey, custom, nymID, accountID, text)
    , chain_(chain)
    , txid_(txid)
    , amount_(amount)
    , memo_(memo)
{
    // NOTE Avoids memory leaks
    extract_custom<proto::PaymentWorkflow>(custom, 0);
    extract_custom<proto::PaymentEvent>(custom, 1);
}

auto BlockchainBalanceItem::Contacts() const noexcept
    -> std::vector<std::string>
{
    auto output = std::vector<std::string>{};

    for (const auto& id : api_.Storage().BlockchainThreadMap(nym_id_, txid_)) {
        if (0 < id->size()) { output.emplace_back(id->str()); }
    }

    return output;
}

auto BlockchainBalanceItem::DisplayAmount() const noexcept -> std::string
{
    return blockchain::internal::Format(chain_, amount_.load());
}

auto BlockchainBalanceItem::Memo() const noexcept -> std::string
{
    sLock lock{shared_lock_};

    return memo_;
}

auto BlockchainBalanceItem::reindex(
    const implementation::AccountActivitySortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    using Transaction = opentxs::blockchain::block::bitcoin::Transaction;

    extract_custom<proto::PaymentWorkflow>(custom, 0);
    extract_custom<proto::PaymentEvent>(custom, 1);
    const auto pTx = extract_custom_ptr<Transaction>(custom, 2);

    OT_ASSERT(pTx);

    const auto& tx = *pTx;
    auto output = BalanceItem::reindex(key, custom);
    const auto chain = extract_custom<blockchain::Type>(custom, 3);
    const auto txid = extract_custom<OTData>(custom, 5);
    const auto amount = tx.NetBalanceChange(Widget::api_.Blockchain(), nym_id_);
    const auto memo = tx.Memo(Widget::api_.Blockchain());
    const auto text = extract_custom<std::string>(custom, 4);

    OT_ASSERT(chain_ == chain);
    OT_ASSERT(txid_ == txid);

    const auto oldAmount = amount_.exchange(amount);

    if (oldAmount != amount) { output |= true; }

    eLock lock{shared_lock_};

    if (memo_ != memo) {
        memo_ = memo;
        output |= true;
    }

    if (text_ != text) {
        text_ = text;
        output |= true;
    }

    return output;
}

auto BlockchainBalanceItem::UUID() const noexcept -> std::string
{
    return blockchain::HashToNumber(txid_);
}
}  // namespace opentxs::ui::implementation
