// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/PaymentItem.cpp"

#pragma once

#include <memory>
#include <string>
#include <thread>

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

class OTPayment;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
class PaymentItem final : public ActivityThreadItem
{
public:
    auto Amount() const noexcept -> opentxs::Amount final;
    auto Deposit() const noexcept -> bool final;
    auto DisplayAmount() const noexcept -> std::string final;
    auto Memo() const noexcept -> std::string final;

    PaymentItem(
        const ActivityThreadInternalInterface& parent,
        const api::client::internal::Manager& api,
        const identifier::Nym& nymID,
        const ActivityThreadRowID& rowID,
        const ActivityThreadSortKey& sortKey,
        CustomData& custom) noexcept;
    ~PaymentItem();

private:
    std::string display_amount_;
    std::string memo_;
    opentxs::Amount amount_;
    std::unique_ptr<std::thread> load_;
    std::shared_ptr<const OTPayment> payment_;

    void load() noexcept;

    PaymentItem() = delete;
    PaymentItem(const PaymentItem&) = delete;
    PaymentItem(PaymentItem&&) = delete;
    auto operator=(const PaymentItem&) -> PaymentItem& = delete;
    auto operator=(PaymentItem&&) -> PaymentItem& = delete;
};
}  // namespace opentxs::ui::implementation
