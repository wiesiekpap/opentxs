// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "ui/activitythread/ActivityThread.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <future>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

#include "Proto.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/protobuf/StorageThread.pb.h"
#include "opentxs/protobuf/StorageThreadItem.pb.h"
#if OT_QT
#include "opentxs/ui/qt/ActivityThread.hpp"
#endif  // OT_QT
#include "ui/base/List.hpp"

template class std::
    tuple<opentxs::OTIdentifier, opentxs::StorageBox, opentxs::OTIdentifier>;

#define OT_METHOD "opentxs::ui::implementation::ActivityThread::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto ActivityThreadModel(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::implementation::ActivityThread>
{
    using ReturnType = ui::implementation::ActivityThread;

    return std::make_unique<ReturnType>(api, nymID, threadID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ActivityThread::ActivityThread(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const SimpleCallback& cb) noexcept
    : ActivityThreadList(
          api,
          nymID,
          cb,
          false
#if OT_QT
          ,
          Roles{
              {ActivityThreadQt::IntAmountRole, "intamount"},
              {ActivityThreadQt::StringAmountRole, "stramount"},
              {ActivityThreadQt::LoadingRole, "loading"},
              {ActivityThreadQt::MemoRole, "memo"},
              {ActivityThreadQt::PendingRole, "pending"},
              {ActivityThreadQt::PolarityRole, "polarity"},
              {ActivityThreadQt::TextRole, "text"},
              {ActivityThreadQt::TimeRole, "time"},
              {ActivityThreadQt::TypeRole, "type"},
          },
          6
#endif
          )
    , Worker(api, std::chrono::milliseconds{100})
    , threadID_(threadID)
    , contacts_()
    , participants_()
    , display_name_()
    , payment_codes_()
    , can_message_(std::nullopt)
    , draft_()
    , draft_tasks_()
    , callbacks_()
{
    init();
    init_executor({
        api.Activity().ThreadPublisher(primary_id_),
        api.Endpoints().ContactUpdate(),
        api.Endpoints().Messagability(),
        api.Endpoints().TaskComplete(),
    });
    pipeline_->Push(MakeWork(Work::init));
}

auto ActivityThread::calculate_display_name() const noexcept -> std::string
{
    auto names = std::set<std::string>{};

    for (const auto& contactID : contacts_) {
        names.emplace(Widget::api_.Contacts().ContactName(contactID));
    }

    return comma(names);
}

auto ActivityThread::calculate_participants() const noexcept -> std::string
{
    auto ids = std::set<std::string>{};

    for (const auto& id : contacts_) { ids.emplace(id->str()); }

    return comma(ids);
}

auto ActivityThread::CanMessage() const noexcept -> bool
{
    return can_message();
}

auto ActivityThread::can_message() const noexcept -> bool
{
    wait_for_startup();
    auto lock = rLock{recursive_lock_};

    if (false == can_message_.has_value()) {
        trigger();

        return false;
    }

    const auto value = can_message_.value();

    return Messagability::READY == value;
}

auto ActivityThread::ClearCallbacks() const noexcept -> void
{
    Widget::ClearCallbacks();
    auto lock = rLock{recursive_lock_};
    callbacks_ = std::nullopt;
}

auto ActivityThread::comma(const std::set<std::string>& list) const noexcept
    -> std::string
{
    auto stream = std::ostringstream{};

    for (const auto& item : list) {
        stream << item;
        stream << ", ";
    }

    auto output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 2, 2); }

    return output;
}

auto ActivityThread::construct_row(
    const ActivityThreadRowID& id,
    const ActivityThreadSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    const auto& box = std::get<1>(id);

    switch (box) {
        case StorageBox::MAILINBOX:
        case StorageBox::MAILOUTBOX: {
            return factory::MailItem(
                *this, Widget::api_, primary_id_, id, index, custom);
        }
        case StorageBox::DRAFT: {
            return factory::MailItem(
                *this,
                Widget::api_,

                primary_id_,
                id,
                index,
                custom,
                false,
                true);
        }
        case StorageBox::INCOMINGCHEQUE:
        case StorageBox::OUTGOINGCHEQUE: {
            return factory::PaymentItem(
                *this, Widget::api_, primary_id_, id, index, custom);
        }
        case StorageBox::PENDING_SEND: {
            return factory::PendingSend(
                *this, Widget::api_, primary_id_, id, index, custom);
        }
#if OT_BLOCKCHAIN
        case StorageBox::BLOCKCHAIN: {
            return factory::BlockchainActivityThreadItem(
                *this, Widget::api_, primary_id_, id, index, custom);
        }
#endif  // OT_BLOCKCHAIN
        case StorageBox::SENTPEERREQUEST:
        case StorageBox::INCOMINGPEERREQUEST:
        case StorageBox::SENTPEERREPLY:
        case StorageBox::INCOMINGPEERREPLY:
        case StorageBox::FINISHEDPEERREQUEST:
        case StorageBox::FINISHEDPEERREPLY:
        case StorageBox::PROCESSEDPEERREQUEST:
        case StorageBox::PROCESSEDPEERREPLY:
        case StorageBox::UNKNOWN:
        default: {
            OT_FAIL
        }
    }
}

auto ActivityThread::DisplayName() const noexcept -> std::string
{
    wait_for_startup();
    auto lock = rLock{recursive_lock_};

    return display_name_;
}

auto ActivityThread::GetDraft() const noexcept -> std::string
{
    auto lock = rLock{recursive_lock_};

    return draft_;
}

auto ActivityThread::load_thread(const proto::StorageThread& thread) noexcept
    -> void
{
    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Loading ")(thread.item().size())(" items.")
        .Flush();

    for (const auto& id : thread.participant()) {
        auto& contacts = const_cast<std::set<OTIdentifier>&>(contacts_);
        contacts.emplace(Widget::api_.Factory().Identifier(id));
    }

    for (const auto& item : thread.item()) { process_item(item); }
}

auto ActivityThread::new_thread() noexcept -> void
{
    auto& contacts = const_cast<std::set<OTIdentifier>&>(contacts_);
    contacts.emplace(threadID_);
}

auto ActivityThread::Participants() const noexcept -> std::string
{
    wait_for_startup();
    auto lock = rLock{recursive_lock_};

    return participants_;
}

auto ActivityThread::Pay(
    const std::string& amount,
    const Identifier& sourceAccount,
    const std::string& memo,
    const PaymentType type) const noexcept -> bool
{
    const auto& unitID = Widget::api_.Storage().AccountContract(sourceAccount);

    if (unitID->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid account: (")(sourceAccount)(")")
            .Flush();

        return false;
    }

    try {
        const auto contract = Widget::api_.Wallet().UnitDefinition(unitID);
        auto value = Amount{0};
        const auto converted =
            contract->StringToAmountLocale(value, amount, "", "");

        if (false == converted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error parsing amount (")(amount)(")")
                .Flush();

            return false;
        }

        return Pay(value, sourceAccount, memo, type);
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Missing unit definition (")(unitID)(")")
            .Flush();

        return false;
    }
}

auto ActivityThread::Pay(
    const Amount amount,
    const Identifier& sourceAccount,
    const std::string& memo,
    const PaymentType type) const noexcept -> bool
{
    wait_for_startup();

    if (0 >= amount) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid amount: (")(amount)(")")
            .Flush();

        return false;
    }

    switch (type) {
        case PaymentType::Cheque: {
            return send_cheque(amount, sourceAccount, memo);
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported payment type: (")(static_cast<int>(type))(")")
                .Flush();

            return false;
        }
    }
}

auto ActivityThread::PaymentCode(
    const contact::ContactItemType currency) const noexcept -> std::string
{
    wait_for_startup();
    auto lock = rLock{recursive_lock_};

    try {

        return payment_codes_.at(currency);
    } catch (...) {

        return {};
    }
}

auto ActivityThread::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    if ((0 == contacts_.size()) && (Work::init != work)) { return; }

    switch (work) {
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::thread: {
            process_thread(in);
        } break;
        case Work::otx: {
            process_otx(in);
        } break;
        case Work::messagability: {
            process_messagability(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto ActivityThread::process_contact(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    auto contactID = [&] {
        auto out = Widget::api_.Factory().Identifier();
        out->Assign(body.at(1).Bytes());

        return out;
    }();

    OT_ASSERT(false == contactID->empty())

    if (0 == contacts_.count(contactID)) { return; }

    const auto changed = update_display_name();

    if (changed) {
        refresh_thread();
        UpdateNotify();
    }

    trigger();
}

auto ActivityThread::process_item(const proto::StorageThreadItem& item) noexcept
    -> ActivityThreadRowID
{
    const auto id = ActivityThreadRowID{
        Widget::api_.Factory().Identifier(item.id()),
        static_cast<StorageBox>(item.box()),
        Widget::api_.Factory().Identifier(item.account())};
    auto& [itemID, box, account] = id;
    const auto key =
        ActivityThreadSortKey{std::chrono::seconds(item.time()), item.index()};
    auto custom = CustomData{new std::string};

    switch (box) {
        case StorageBox::BLOCKCHAIN: {
            const auto chain = static_cast<blockchain::Type>(item.chain());
            custom.emplace_back(new blockchain::Type{chain});
            custom.emplace_back(new std::string{item.txid()});
        } break;
        default: {
        }
    }

    add_item(id, key, custom);

    OT_ASSERT(verify_empty(custom));

    return id;
}

auto ActivityThread::process_messagability(const Message& message) noexcept
    -> void
{
    const auto body = message.Body();

    OT_ASSERT(3 < body.size());

    const auto nym = [&] {
        auto output = Widget::api_.Factory().NymID();
        output->Assign(body.at(1).Bytes());

        return output;
    }();

    if (nym != primary_id_) { return; }

    const auto contact = [&] {
        auto output = Widget::api_.Factory().Identifier();
        output->Assign(body.at(2).Bytes());

        return output;
    }();

    if (0 == contacts_.count(contact)) { return; }

    if (update_messagability(body.at(3).as<Messagability>())) {
        UpdateNotify();
    }
}

auto ActivityThread::process_otx(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    const auto id = body.at(1).as<api::client::OTX::TaskID>();
    auto done = [&] {
        auto output = std::optional<ActivityThreadRowID>{};
        auto lock = rLock{recursive_lock_};
        auto it = draft_tasks_.find(id);

        if (draft_tasks_.end() == it) { return output; }

        auto& [taskID, task] = *it;
        auto& [rowID, backgroundTask] = task;
        auto& future = std::get<1>(backgroundTask);
        const auto [status, reply] = future.get();

        if (otx::LastReplyStatus::MessageSuccess == status) {
            LogDebug(OT_METHOD)(__FUNCTION__)(
                ": Task ")(taskID)(" completed successfully")
                .Flush();
        } else {
            // TODO consider taking some action in response to failed sends.
        }

        output = rowID;
        draft_tasks_.erase(it);

        return output;
    }();

    if (done.has_value()) {
        auto lock = rLock{recursive_lock_};
        delete_item(lock, done.value());
    }
}

auto ActivityThread::process_thread(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto threadID = [&] {
        auto output = Widget::api_.Factory().Identifier();
        output->Assign(body.at(1).Bytes());

        return output;
    }();

    OT_ASSERT(false == threadID->empty())

    if (threadID_ != threadID) { return; }

    refresh_thread();
}

auto ActivityThread::refresh_thread() noexcept -> void
{
    auto thread = proto::StorageThread{};
    auto loaded =
        Widget::api_.Activity().Thread(primary_id_, threadID_, thread);

    OT_ASSERT(loaded)

    auto active = std::set<ActivityThreadRowID>{};

    for (const auto& item : thread.item()) {
        const auto itemID = process_item(item);
        active.emplace(itemID);
    }

    const auto drafts = [&] {
        auto lock = rLock{recursive_lock_};

        return draft_tasks_;
    }();

    for (const auto& [taskID, job] : drafts) {
        const auto& [rowid, future] = job;
        active.emplace(rowid);
    }

    delete_inactive(active);
}

auto ActivityThread::send_cheque(
    const Amount amount,
    const Identifier& sourceAccount,
    const std::string& memo) const noexcept -> bool
{
    if (false == validate_account(sourceAccount)) { return false; }

    if (1 < contacts_.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Sending to multiple recipient not yet supported.")
            .Flush();

        return false;
    }

    auto displayAmount = std::string{};

    try {
        const auto contract = Widget::api_.Wallet().UnitDefinition(
            Widget::api_.Storage().AccountContract(sourceAccount));
        contract->FormatAmountLocale(amount, displayAmount, ",", ".");
    } catch (...) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to load unit definition contract")
            .Flush();

        return false;
    }

    auto task = make_blank<DraftTask>::value(Widget::api_);
    auto& [id, otx] = task;
    otx = Widget::api_.OTX().SendCheque(
        primary_id_, sourceAccount, *contacts_.cbegin(), amount, memo);
    const auto taskID = std::get<0>(otx);

    if (0 == taskID) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to queue payment for sending.")
            .Flush();

        return false;
    }

    id = ActivityThreadRowID{
        Identifier::Random(), StorageBox::PENDING_SEND, Identifier::Factory()};
    const auto key = ActivityThreadSortKey{Clock::now(), 0};
    auto custom = CustomData{
        new std::string{"Sending cheque"},
        new Amount{amount},
        new std::string{displayAmount},
        new std::string{memo}};
    {
        auto lock = rLock{recursive_lock_};
        const_cast<ActivityThread&>(*this).add_item(id, key, custom);
        draft_tasks_.try_emplace(taskID, std::move(task));
    }

    OT_ASSERT(verify_empty(custom));

    return true;
}

auto ActivityThread::SendDraft() const noexcept -> bool
{
    wait_for_startup();
    {
        auto lock = rLock{recursive_lock_};

        if (draft_.empty()) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": No draft message to send.")
                .Flush();

            return false;
        }

        auto task = make_blank<DraftTask>::value(Widget::api_);
        auto& [id, otx] = task;
        otx = Widget::api_.OTX().MessageContact(
            primary_id_, *contacts_.begin(), draft_);
        const auto taskID = std::get<0>(otx);

        if (0 == taskID) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to queue message for sending.")
                .Flush();

            return false;
        }

        id = ActivityThreadRowID{
            Identifier::Random(), StorageBox::DRAFT, Identifier::Factory()};
        const ActivityThreadSortKey key{Clock::now(), 0};
        auto custom = CustomData{new std::string(draft_)};
        const_cast<ActivityThread&>(*this).add_item(id, key, custom);
        draft_tasks_.try_emplace(taskID, std::move(task));
        draft_.clear();

        OT_ASSERT(verify_empty(custom));
    }

    return true;
}

auto ActivityThread::SetCallbacks(Callbacks&& cb) noexcept -> void
{
    auto lock = rLock{recursive_lock_};
    callbacks_ = std::move(cb);

    if (callbacks_ && callbacks_.value().general_) {
        SetCallback(callbacks_.value().general_);
    }
}

auto ActivityThread::SetDraft(const std::string& draft) const noexcept -> bool
{
    if (draft.empty()) { return false; }

    wait_for_startup();
    auto lock = rLock{recursive_lock_};

    if (false == can_message()) { return false; }

    draft_ = draft;

    if (callbacks_ && callbacks_.value().draft_) {
        callbacks_.value().draft_();
    }

    return true;
}

auto ActivityThread::set_participants() noexcept -> void
{
    auto& participants = const_cast<std::string&>(participants_);
    participants = calculate_participants();
}

auto ActivityThread::startup() noexcept -> void
{
    auto thread = proto::StorageThread{};
    auto loaded =
        Widget::api_.Activity().Thread(primary_id_, threadID_, thread);

    if (loaded) {
        load_thread(thread);
    } else {
        new_thread();
    }

    set_participants();
    auto changed = update_display_name();
    changed |= update_payment_codes();

    if (changed) { UpdateNotify(); }

    finish_startup();
    trigger();
}

auto ActivityThread::state_machine() noexcept -> bool
{
    auto again{false};

    for (const auto& id : contacts_) {
        const auto value =
            Widget::api_.OTX().CanMessage(primary_id_, id, false);

        switch (value) {
            case Messagability::READY: {

                return false;
            }
            case Messagability::UNREGISTERED: {

                again |= true;
            } break;
            default: {
            }
        }
    }

    return again;
}

auto ActivityThread::ThreadID() const noexcept -> std::string
{
    return threadID_->str();
}

auto ActivityThread::update_display_name() noexcept -> bool
{
    auto name = calculate_display_name();
    auto changed{false};

    {
        auto lock = rLock{recursive_lock_};
        changed = (display_name_ != name);
        display_name_.swap(name);

        if (changed && callbacks_ && callbacks_.value().display_name_) {
            callbacks_.value().display_name_();
        }
    }

    return changed;
}

auto ActivityThread::update_messagability(Messagability value) noexcept -> bool
{
    const auto changed = [&] {
        auto lock = rLock{recursive_lock_};
        auto output = !can_message_.has_value();
        const auto old = can_message_.value_or(value);
        can_message_ = value;
        output |= (old != value);

        return output;
    }();

    {
        auto lock = rLock{recursive_lock_};

        if (changed && callbacks_ && callbacks_.value().messagability_) {
            callbacks_.value().messagability_(
                Messagability::READY == can_message_.value());
        }
    }

    return changed;
}

auto ActivityThread::update_payment_codes() noexcept -> bool
{
    auto map = std::map<contact::ContactItemType, std::string>{};

    if (1 != contacts_.size()) { OT_FAIL; }

    const auto contact = Widget::api_.Contacts().Contact(*contacts_.cbegin());

    OT_ASSERT(contact);

    for (const auto chain : blockchain::DefinedChains()) {
        auto type = Translate(chain);
        auto code = contact->PaymentCode(type);

        if (code.empty()) { continue; }

        map.try_emplace(type, std::move(code));
    }

    auto changed{false};
    {
        auto lock = rLock{recursive_lock_};
        changed = (payment_codes_ != map);
        payment_codes_.swap(map);
    }

    return changed;
}

auto ActivityThread::validate_account(
    const Identifier& sourceAccount) const noexcept -> bool
{
    const auto owner = Widget::api_.Storage().AccountOwner(sourceAccount);

    if (owner->empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Invalid account id: (")(sourceAccount)(")")
            .Flush();

        return false;
    }

    if (primary_id_ != owner) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Account ")(sourceAccount)(" is not owned by nym ")(primary_id_)
            .Flush();

        return false;
    }

    return true;
}

ActivityThread::~ActivityThread()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
