// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <tuple>

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
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class PendingSend final : public ActivityThreadItem
{
public:
    static auto extract(CustomData& custom) noexcept
        -> std::tuple<opentxs::Amount, std::string, std::string>;

    auto Amount() const noexcept -> opentxs::Amount final;
    auto Deposit() const noexcept -> bool final { return false; }
    auto DisplayAmount() const noexcept -> std::string final;
    auto Memo() const noexcept -> std::string final;

    PendingSend(
        const ActivityThreadInternalInterface& parent,
        const api::client::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom,
        opentxs::Amount amount,
        std::string&& display,
        std::string&& memo) noexcept;
    ~PendingSend() final = default;

private:
    opentxs::Amount amount_;
    std::string display_amount_;
    std::string memo_;

    auto reindex(const ActivityThreadSortKey& key, CustomData& custom) noexcept
        -> bool final;

    PendingSend() = delete;
    PendingSend(const PendingSend&) = delete;
    PendingSend(PendingSend&&) = delete;
    auto operator=(const PendingSend&) -> PendingSend& = delete;
    auto operator=(PendingSend&&) -> PendingSend& = delete;
};
}  // namespace opentxs::ui::implementation
