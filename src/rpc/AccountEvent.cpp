// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/rpc/AccountEvent.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>

#include "internal/rpc/RPC.hpp"
#include "opentxs/protobuf/AccountEvent.pb.h"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/rpc/AccountEventType.hpp"

namespace opentxs::rpc
{
constexpr auto default_version_ = VersionNumber{2};

struct AccountEvent::Imp {
    const VersionNumber version_;
    const std::string account_;
    const AccountEventType type_;
    const std::string contact_;
    const std::string workflow_;
    const std::string amount_formatted_;
    const std::string pending_formatted_;
    const Amount amount_;
    const Amount pending_;
    const opentxs::Time time_;
    const std::string memo_;
    const std::string uuid_;
    const int state_;

    Imp(const VersionNumber version,
        const std::string& account,
        AccountEventType type,
        const std::string& contact,
        const std::string& workflow,
        const std::string& amountF,
        const std::string& pendingF,
        Amount amount,
        Amount pending,
        opentxs::Time time,
        const std::string& memo,
        const std::string& uuid,
        int state) noexcept(false)
        : version_(version)
        , account_(account)
        , type_(type)
        , contact_(contact)
        , workflow_(workflow)
        , amount_formatted_(amountF)
        , pending_formatted_(pendingF)
        , amount_(amount)
        , pending_(pending)
        , time_(time)
        , memo_(memo)
        , uuid_(uuid)
        , state_(state)
    {
        // FIXME validate member variables
    }
    Imp() noexcept
        : Imp(0,
              "",
              AccountEventType::error,
              "",
              "",
              "",
              "",
              0,
              0,
              {},
              "",
              "",
              0)
    {
    }
    Imp(const Imp& rhs) noexcept
        : Imp(rhs.version_,
              rhs.account_,
              rhs.type_,
              rhs.contact_,
              rhs.workflow_,
              rhs.amount_formatted_,
              rhs.pending_formatted_,
              rhs.amount_,
              rhs.pending_,
              rhs.time_,
              rhs.memo_,
              rhs.uuid_,
              rhs.state_)
    {
    }

private:
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};

AccountEvent::AccountEvent(
    const std::string& account,
    AccountEventType type,
    const std::string& contact,
    const std::string& workflow,
    const std::string& amountS,
    const std::string& pendingS,
    Amount amount,
    Amount pending,
    opentxs::Time time,
    const std::string& memo,
    const std::string& uuid,
    int state) noexcept(false)
    : imp_(std::make_unique<Imp>(
               default_version_,
               account,
               type,
               contact,
               workflow,
               amountS,
               pendingS,
               amount,
               pending,
               time,
               memo,
               uuid,
               state)
               .release())
{
}

AccountEvent::AccountEvent(const proto::AccountEvent& in) noexcept(false)
    : imp_(std::make_unique<Imp>(
               in.version(),
               in.id(),
               translate(in.type()),
               in.contact(),
               in.workflow(),
               in.amountformatted(),
               in.pendingamountformatted(),
               in.amount(),
               in.pendingamount(),
               Clock::from_time_t(in.timestamp()),
               in.memo(),
               in.uuid(),
               in.state())
               .release())
{
}

AccountEvent::AccountEvent(const AccountEvent& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_).release())
{
    assert(nullptr != imp_);
}

AccountEvent::AccountEvent(AccountEvent&& rhs) noexcept
    : imp_(std::make_unique<Imp>().release())
{
    std::swap(imp_, rhs.imp_);

    assert(nullptr != imp_);
}

auto AccountEvent::AccountID() const noexcept -> const std::string&
{
    return imp_->account_;
}

auto AccountEvent::ConfirmedAmount() const noexcept -> Amount
{
    return imp_->amount_;
}

auto AccountEvent::ConfirmedAmount_str() const noexcept -> const std::string&
{
    return imp_->amount_formatted_;
}

auto AccountEvent::ContactID() const noexcept -> const std::string&
{
    return imp_->contact_;
}

auto AccountEvent::Memo() const noexcept -> const std::string&
{
    return imp_->memo_;
}

auto AccountEvent::PendingAmount() const noexcept -> Amount
{
    return imp_->pending_;
}

auto AccountEvent::PendingAmount_str() const noexcept -> const std::string&
{
    return imp_->pending_formatted_;
}

auto AccountEvent::Serialize(proto::AccountEvent& dest) const noexcept -> bool
{
    const auto& imp = *imp_;
    dest.set_version(std::max(default_version_, imp.version_));
    dest.set_id(imp.account_);
    dest.set_type(translate(imp.type_));
    dest.set_contact(imp.contact_);
    dest.set_workflow(imp.workflow_);
    dest.set_amount(imp.amount_);
    dest.set_pendingamount(imp.pending_);
    dest.set_timestamp(Clock::to_time_t(imp.time_));
    dest.set_memo(imp.memo_);
    dest.set_uuid(imp.uuid_);
    dest.set_state(static_cast<proto::PaymentWorkflowState>(imp.state_));
    dest.set_amountformatted(imp.amount_formatted_);
    dest.set_pendingamountformatted(imp.pending_formatted_);

    return true;
}

auto AccountEvent::State() const noexcept -> int { return imp_->state_; }

auto AccountEvent::Timestamp() const noexcept -> Time { return imp_->time_; }

auto AccountEvent::Type() const noexcept -> AccountEventType
{
    return imp_->type_;
}

auto AccountEvent::UUID() const noexcept -> const std::string&
{
    return imp_->uuid_;
}

auto AccountEvent::WorkflowID() const noexcept -> const std::string&
{
    return imp_->workflow_;
}

AccountEvent::~AccountEvent()
{
    std::unique_ptr<Imp>{imp_}.reset();
    imp_ = nullptr;
}
}  // namespace opentxs::rpc
