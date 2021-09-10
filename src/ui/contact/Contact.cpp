// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "ui/contact/Contact.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <utility>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/ContactSectionName.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "ui/base/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::Contact::"

namespace opentxs::factory
{
auto ContactModel(
    const api::client::Manager& api,
    const ui::implementation::ContactPrimaryID& contactID,
    const SimpleCallback& cb) noexcept -> std::unique_ptr<ui::internal::Contact>
{
    using ReturnType = ui::implementation::Contact;

    return std::make_unique<ReturnType>(api, contactID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
const std::set<contact::ContactSectionName> Contact::allowed_types_{
    contact::ContactSectionName::Communication,
    contact::ContactSectionName::Profile};

const std::map<contact::ContactSectionName, int> Contact::sort_keys_{
    {contact::ContactSectionName::Communication, 0},
    {contact::ContactSectionName::Profile, 1}};

Contact::Contact(
    const api::client::Manager& api,
    const Identifier& contactID,
    const SimpleCallback& cb) noexcept
    : ContactType(api, contactID, cb, false)
    , listeners_({
          {api_.Endpoints().ContactUpdate(),
           new MessageProcessor<Contact>(&Contact::process_contact)},
      })
    , callbacks_()
    , name_(api_.Contacts().ContactName(contactID))
    , payment_code_()
{
    // NOTE nym_id_ is actually the contact id
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&Contact::startup, this));

    OT_ASSERT(startup_)
}

auto Contact::check_type(const contact::ContactSectionName type) noexcept
    -> bool
{
    return 1 == allowed_types_.count(type);
}

auto Contact::ClearCallbacks() const noexcept -> void
{
    Widget::ClearCallbacks();
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = {};
}

auto Contact::construct_row(
    const ContactRowID& id,
    const ContactSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ContactSectionWidget(*this, api_, id, index, custom);
}

auto Contact::ContactID() const noexcept -> std::string
{
    return primary_id_->str();
}

auto Contact::DisplayName() const noexcept -> std::string
{
    auto lock = rLock{recursive_lock_};

    return name_;
}

auto Contact::PaymentCode() const noexcept -> std::string
{
    auto lock = rLock{recursive_lock_};

    return payment_code_;
}

auto Contact::process_contact(const opentxs::Contact& contact) noexcept -> void
{
    {
        const auto name = contact.Label();
        const auto code = contact.PaymentCode();
        auto nameChanged{false};
        auto codeChanged{false};

        {
            auto lock = rLock{recursive_lock_};

            if (name_ != name) {
                name_ = name;
                nameChanged = true;
            }

            if (payment_code_ != code) {
                payment_code_ = code;
                codeChanged = true;
            }
        }

        {
            auto lock = Lock{callbacks_.lock_};
            auto& cb = callbacks_.cb_;

            if (nameChanged && cb.name_) { cb.name_(name.c_str()); }
            if (codeChanged && cb.payment_code_) {
                cb.payment_code_(code.c_str());
            }
        }

        if (nameChanged || codeChanged) { UpdateNotify(); }
    }

    auto active = std::set<ContactRowID>{};
    const auto data = contact.Data();

    if (data) {
        for (const auto& section : *data) {
            auto& type = section.first;

            if (check_type(type)) {
                auto custom =
                    CustomData{new opentxs::ContactSection(*section.second)};
                add_item(type, sort_key(type), custom);
                active.emplace(type);
            }
        }
    }

    delete_inactive(active);
}

auto Contact::process_contact(const Message& message) noexcept -> void
{
    wait_for_startup();

    const auto body = message.Body();

    OT_ASSERT(1 < body.size());

    const auto& id = body.at(1);
    auto contactID = Widget::api_.Factory().Identifier();
    contactID->Assign(id.Bytes());

    OT_ASSERT(false == contactID->empty())

    if (contactID != primary_id_) { return; }

    const auto contact = api_.Contacts().Contact(contactID);

    OT_ASSERT(contact)

    process_contact(*contact);
}

auto Contact::SetCallbacks(Callbacks&& cb) noexcept -> void
{
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = std::move(cb);
}

auto Contact::sort_key(const contact::ContactSectionName type) noexcept -> int
{
    return sort_keys_.at(type);
}

auto Contact::startup() noexcept -> void
{
    LogVerbose(OT_METHOD)(__func__)(": Loading contact ")(primary_id_).Flush();
    const auto contact = api_.Contacts().Contact(primary_id_);

    OT_ASSERT(contact)

    process_contact(*contact);
    finish_startup();
}

Contact::~Contact()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
