// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/MailItem.cpp"

#pragma once

#include <memory>
#include <thread>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
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
class MailItem final : public ActivityThreadItem
{
public:
    MailItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom,
        const bool loading,
        const bool pending) noexcept;
    MailItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom) noexcept;
    ~MailItem();

private:
    std::unique_ptr<std::thread> load_;

    void load() noexcept;

    MailItem() = delete;
    MailItem(const MailItem&) = delete;
    MailItem(MailItem&&) = delete;
    auto operator=(const MailItem&) -> MailItem& = delete;
    auto operator=(MailItem&&) -> MailItem& = delete;
};
}  // namespace opentxs::ui::implementation
