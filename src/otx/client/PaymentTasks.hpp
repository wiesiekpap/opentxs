// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <mutex>

#include "core/StateMachine.hpp"
#include "internal/api/client/Client.hpp"
#include "internal/otx/client/Client.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/ext/OTPayment.hpp"
#include "otx/client/DepositPayment.hpp"

namespace opentxs::otx::client::implementation
{
class PaymentTasks final : public opentxs::internal::StateMachine
{
public:
    using BackgroundTask = api::client::OTX::BackgroundTask;

    auto GetAccountLock(const identifier::UnitDefinition& unit) -> std::mutex&;
    auto Queue(const DepositPaymentTask& task) -> BackgroundTask;

    PaymentTasks(client::internal::StateMachine& parent);
    ~PaymentTasks() final = default;

private:
    using Future = api::client::OTX::Future;
    using TaskMap = std::map<OTIdentifier, implementation::DepositPayment>;

    static auto error_task() -> BackgroundTask;

    client::internal::StateMachine& parent_;
    TaskMap tasks_;
    std::mutex unit_lock_;
    std::map<OTUnitID, std::mutex> account_lock_;

    auto cleanup() -> bool;
    auto get_payment_id(const OTPayment& payment) const -> OTIdentifier;

    PaymentTasks() = delete;
};
}  // namespace opentxs::otx::client::implementation
