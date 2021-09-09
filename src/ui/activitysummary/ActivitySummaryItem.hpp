// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <tuple>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "ui/base/Row.hpp"

class QVariant;

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

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

namespace proto
{
class StorageThread;
}  // namespace proto

namespace ui
{
class ActivitySummaryItem;
}  // namespace ui

class Flag;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ActivitySummaryItemRow =
    Row<ActivitySummaryRowInternal,
        ActivitySummaryInternalInterface,
        ActivitySummaryRowID>;

class ActivitySummaryItem final : public ActivitySummaryItemRow
{
public:
    static auto LoadItemText(
        const api::client::Manager& api,
        const identifier::Nym& nym,
        const CustomData& custom) noexcept -> std::string;

    auto DisplayName() const noexcept -> std::string final;
    auto ImageURI() const noexcept -> std::string final;
    auto Text() const noexcept -> std::string final;
    auto ThreadID() const noexcept -> std::string final;
    auto Timestamp() const noexcept -> Time final;
    auto Type() const noexcept -> StorageBox final;

    ActivitySummaryItem(
        const ActivitySummaryInternalInterface& parent,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const ActivitySummaryRowID& rowID,
        const ActivitySummarySortKey& sortKey,
        CustomData& custom,
        const Flag& running,
        std::string text) noexcept;

    ~ActivitySummaryItem() final;

private:
    // id, box, account, thread
    using ItemLocator =
        std::tuple<std::string, StorageBox, std::string, OTIdentifier>;

    const Flag& running_;
    const OTNymID nym_id_;
    ActivitySummarySortKey key_;
    std::string& display_name_;
    std::string text_;
    StorageBox type_;
    Time time_;
    std::unique_ptr<std::thread> newest_item_thread_;
    UniqueQueue<ItemLocator> newest_item_;
    std::atomic<int> next_task_id_;
    std::atomic<bool> break_;

    auto find_text(const PasswordPrompt& reason, const ItemLocator& locator)
        const noexcept -> std::string;
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto get_text() noexcept -> void;
    auto reindex(const ActivitySummarySortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(CustomData& custom) noexcept -> void;

    ActivitySummaryItem(const ActivitySummaryItem&) = delete;
    ActivitySummaryItem(ActivitySummaryItem&&) = delete;
    auto operator=(const ActivitySummaryItem&) -> ActivitySummaryItem& = delete;
    auto operator=(ActivitySummaryItem&&) -> ActivitySummaryItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ActivitySummaryItem>;
