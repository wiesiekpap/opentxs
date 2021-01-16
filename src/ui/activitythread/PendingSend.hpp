// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/PendingSend.cpp"

#pragma once

#include <string>

#include "1_Internal.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
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
class PendingSend final : public ActivityThreadItem
{
public:
    auto Amount() const noexcept -> opentxs::Amount final { return amount_; }
    auto Deposit() const noexcept -> bool final { return false; }
    auto DisplayAmount() const noexcept -> std::string final
    {
        return display_amount_;
    }
    auto Memo() const noexcept -> std::string final { return memo_; }

    PendingSend(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom) noexcept;
    ~PendingSend() = default;

private:
    opentxs::Amount amount_;
    std::string display_amount_;
    std::string memo_;

    PendingSend() = delete;
    PendingSend(const PendingSend&) = delete;
    PendingSend(PendingSend&&) = delete;
    auto operator=(const PendingSend&) -> PendingSend& = delete;
    auto operator=(PendingSend&&) -> PendingSend& = delete;
};
}  // namespace opentxs::ui::implementation
