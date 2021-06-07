// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>
#include <thread>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "ui/activitythread/ActivityThreadItem.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class BlockchainActivityThreadItem final : public ActivityThreadItem
{
public:
    auto Amount() const noexcept -> opentxs::Amount final { return amount_; }
    auto DisplayAmount() const noexcept -> std::string final
    {
        return display_amount_;
    }
    auto Memo() const noexcept -> std::string final { return memo_; }

    BlockchainActivityThreadItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom,
        OTData&& txid,
        const opentxs::Amount amount,
        std::string&& displayAmount,
        const std::string& memo) noexcept;

    ~BlockchainActivityThreadItem() final = default;

private:
    const OTData txid_;
    const std::string display_amount_;
    const std::string memo_;
    const opentxs::Amount amount_;

    BlockchainActivityThreadItem() = delete;
    BlockchainActivityThreadItem(const BlockchainActivityThreadItem&) = delete;
    BlockchainActivityThreadItem(BlockchainActivityThreadItem&&) = delete;
    auto operator=(const BlockchainActivityThreadItem&)
        -> BlockchainActivityThreadItem& = delete;
    auto operator=(BlockchainActivityThreadItem&&)
        -> BlockchainActivityThreadItem& = delete;
};
}  // namespace opentxs::ui::implementation
