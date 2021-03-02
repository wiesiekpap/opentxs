// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "ui/messagablelist/MessagableList.hpp"  // IWYU pragma: associated

#include <future>
#include <list>
#include <memory>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#if OT_QT
#include "opentxs/ui/qt/MessagableList.hpp"
#endif  // OT_QT
#include "ui/base/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::MessagableList::"

namespace opentxs::factory
{
auto MessagableListModel(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::implementation::MessagableList>
{
    using ReturnType = ui::implementation::MessagableList;

    return std::make_unique<ReturnType>(api, nymID, cb);
}

#if OT_QT
auto MessagableListQtModel(ui::implementation::MessagableList& parent) noexcept
    -> std::unique_ptr<ui::MessagableListQt>
{
    using ReturnType = ui::MessagableListQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(MessagableListQt, implementation::MessagableList)
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
MessagableList::MessagableList(
    const api::client::internal::Manager& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : MessagableListList(
          api,
          nymID,
          cb,
          false
#if OT_QT
          ,
          Roles{
              {MessagableListQt::ContactIDRole, "id"},
              {MessagableListQt::SectionRole, "section"}},
          1
#endif
          )
    , Worker(api, {})
    , owner_contact_id_(Widget::api_.Contacts().ContactID(nymID))
{
    init();
    init_executor(
        {api.Endpoints().ContactUpdate(), api.Endpoints().NymDownload()});
    pipeline_->Push(MakeWork(Work::init));
}

auto MessagableList::construct_row(
    const MessagableListRowID& id,
    const MessagableListSortKey& index,
    CustomData&) const noexcept -> RowPointer
{
    return factory::MessagableListItem(*this, Widget::api_, id, index);
}

auto MessagableList::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = body.at(0).as<Work>();

    switch (work) {
        case Work::contact: {
            process_contact(in);
        } break;
        case Work::nym: {
            process_nym(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto MessagableList::process_contact(
    const MessagableListRowID& id,
    const MessagableListSortKey& key) noexcept -> void
{
    if (owner_contact_id_ == id) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Skipping owner contact ")(id)(
            " (")(key)(")")
            .Flush();

        return;
    } else {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Incoming contact ")(id)(" (")(
            key)(") is not owner contact: (")(owner_contact_id_)(")")
            .Flush();
    }

    switch (Widget::api_.OTX().CanMessage(primary_id_, id, false)) {
        case Messagability::READY:
        case Messagability::MISSING_RECIPIENT:
        case Messagability::UNREGISTERED: {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Messagable contact ")(id)(
                " (")(key)(")")
                .Flush();
            auto custom = CustomData{};
            add_item(id, key, custom);
        } break;
        case Messagability::MISSING_SENDER:
        case Messagability::INVALID_SENDER:
        case Messagability::NO_SERVER_CLAIM:
        case Messagability::CONTACT_LACKS_NYM:
        case Messagability::MISSING_CONTACT:
        default: {
            LogDetail(OT_METHOD)(__FUNCTION__)(
                ": Skipping non-messagable contact ")(id)(" (")(key)(")")
                .Flush();
            delete_item(id);
        }
    }
}

auto MessagableList::process_contact(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    auto contactID = Widget::api_.Factory().Identifier();
    contactID->Assign(id.Bytes());

    OT_ASSERT(false == contactID->empty())

    const auto name = Widget::api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

auto MessagableList::process_nym(const Message& message) noexcept -> void
{
    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    auto nymID = Widget::api_.Factory().NymID();
    nymID->Assign(id.Bytes());

    OT_ASSERT(false == nymID->empty())

    const auto contactID = Widget::api_.Contacts().ContactID(nymID);
    const auto name = Widget::api_.Contacts().ContactName(contactID);
    process_contact(contactID, name);
}

auto MessagableList::startup() noexcept -> void
{
    const auto contacts = Widget::api_.Contacts().ContactList();
    LogDetail(OT_METHOD)(__FUNCTION__)(": Loading ")(contacts.size())(
        " contacts.")
        .Flush();

    for (const auto& [id, alias] : contacts) {
        process_contact(Identifier::Factory(id), alias);
    }

    finish_startup();
}

MessagableList::~MessagableList()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
