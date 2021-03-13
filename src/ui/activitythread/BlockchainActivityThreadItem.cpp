// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/activitythread/BlockchainActivityThreadItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/api/client/Client.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"
#include "ui/base/Widget.hpp"

// #define OT_METHOD
// "opentxs::ui::implementation::BlockchainActivityThreadItem::"

namespace opentxs::factory
{
auto BlockchainActivityThreadItem(
    const ui::implementation::ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const ui::implementation::ActivityThreadRowID& rowID,
    const ui::implementation::ActivityThreadSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ActivityThreadRowInternal>
{
    using ReturnType = ui::implementation::BlockchainActivityThreadItem;

    const auto chain =
        ui::implementation::extract_custom<blockchain::Type>(custom, 1);
    auto txid = ui::implementation::extract_custom<OTData>(custom, 2);
    const auto pTx = api.Blockchain().LoadTransactionBitcoin(txid->asHex());

    OT_ASSERT(pTx);

    const auto& tx = *pTx;
    const auto amount = tx.NetBalanceChange(api.Blockchain(), nymID);
    const auto memo = tx.Memo(api.Blockchain());
    auto* text = static_cast<std::string*>(custom.at(0));

    OT_ASSERT(nullptr != text);

    *text = api.Blockchain().ActivityDescription(nymID, chain, tx);

    return std::make_shared<ReturnType>(
        parent,
        api,
        nymID,
        rowID,
        sortKey,
        custom,
        std::move(txid),
        amount,
        blockchain::internal::Format(chain, amount),
        memo);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainActivityThreadItem::BlockchainActivityThreadItem(
    const ActivityThreadInternalInterface& parent,
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const ActivityThreadRowID& rowID,
    const ActivityThreadSortKey& sortKey,
    CustomData& custom,
    OTData&& txid,
    const opentxs::Amount amount,
    std::string&& displayAmount,
    const std::string& memo) noexcept
    : ActivityThreadItem(
          parent,
          api,
          nymID,
          rowID,
          sortKey,
          custom,
          false,
          false)
    , txid_(std::move(txid))
    , display_amount_(std::move(displayAmount))
    , memo_(memo)
    , amount_(amount)
{
    OT_ASSERT(false == nym_id_.empty())
    OT_ASSERT(false == item_id_.empty())
    OT_ASSERT(false == txid_->empty())
}
}  // namespace opentxs::ui::implementation
