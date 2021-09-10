// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <list>
#include <map>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
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

class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class StorageThread;
class StorageThreadItem;
}  // namespace proto

class Flag;
class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ActivitySummaryList = List<
    ActivitySummaryExternalInterface,
    ActivitySummaryInternalInterface,
    ActivitySummaryRowID,
    ActivitySummaryRowInterface,
    ActivitySummaryRowInternal,
    ActivitySummaryRowBlank,
    ActivitySummarySortKey,
    ActivitySummaryPrimaryID>;

class ActivitySummary final : public ActivitySummaryList
{
public:
    ActivitySummary(
        const api::client::Manager& api,
        const Flag& running,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;

    ~ActivitySummary() final;

private:
    const ListenerDefinitions listeners_;
    const Flag& running_;

    static auto newest_item(
        const Identifier& id,
        const proto::StorageThread& thread,
        CustomData& custom) noexcept -> const proto::StorageThreadItem&;

    auto construct_row(
        const ActivitySummaryRowID& id,
        const ActivitySummarySortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto display_name(const proto::StorageThread& thread) const noexcept
        -> std::string;

    void process_thread(const std::string& threadID) noexcept;
    void process_thread(const Message& message) noexcept;
    void startup() noexcept;

    ActivitySummary() = delete;
    ActivitySummary(const ActivitySummary&) = delete;
    ActivitySummary(ActivitySummary&&) = delete;
    auto operator=(const ActivitySummary&) -> ActivitySummary& = delete;
    auto operator=(ActivitySummary&&) -> ActivitySummary& = delete;
};
}  // namespace opentxs::ui::implementation
