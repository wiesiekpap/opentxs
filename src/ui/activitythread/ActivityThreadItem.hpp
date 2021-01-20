// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "ui/base/Row.hpp"

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

namespace ui
{
class ActivityThreadItem;
}  // namespace ui

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ActivityThreadItemRow =
    Row<ActivityThreadRowInternal,
        ActivityThreadInternalInterface,
        ActivityThreadRowID>;

class ActivityThreadItem : public ActivityThreadItemRow
{
public:
    auto Amount() const noexcept -> opentxs::Amount override { return 0; }
    auto Deposit() const noexcept -> bool override { return false; }
    auto DisplayAmount() const noexcept -> std::string override { return {}; }
    auto Loading() const noexcept -> bool final { return loading_.get(); }
    auto MarkRead() const noexcept -> bool final;
    auto Memo() const noexcept -> std::string override { return {}; }
    auto Pending() const noexcept -> bool final { return pending_.get(); }
    auto Text() const noexcept -> std::string final;
    auto Timestamp() const noexcept -> Time final;
    auto Type() const noexcept -> StorageBox final { return box_; }

    auto reindex(const ActivityThreadSortKey& key, CustomData& custom) noexcept
        -> bool final;

#if OT_QT
    QVariant qt_data(const int column, const int role) const noexcept final;
#endif

    ~ActivityThreadItem() override = default;

protected:
    const identifier::Nym& nym_id_;
    const Time time_;
    const Identifier& item_id_;
    const StorageBox& box_;
    const Identifier& account_id_;
    std::string text_;
    OTFlag loading_;
    OTFlag pending_;

    ActivityThreadItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom,
        const bool loading,
        const bool pending) noexcept;

private:
    ActivityThreadItem() = delete;
    ActivityThreadItem(const ActivityThreadItem&) = delete;
    ActivityThreadItem(ActivityThreadItem&&) = delete;
    auto operator=(const ActivityThreadItem&) -> ActivityThreadItem& = delete;
    auto operator=(ActivityThreadItem&&) -> ActivityThreadItem& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ActivityThreadItem>;
