// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "ui/profile/Profile.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/contact/Contact.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactItemAttribute.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/ContactSectionName.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/ProfileSection.hpp"
#include "ui/base/List.hpp"

template struct std::pair<int, std::string>;

#define OT_METHOD "opentxs::ui::implementation::Profile::"

namespace opentxs::factory
{
auto ProfileModel(
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept -> std::unique_ptr<ui::internal::Profile>
{
    using ReturnType = ui::implementation::Profile;

    return std::make_unique<ReturnType>(api, nymID, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
const std::set<contact::ContactSectionName> Profile::allowed_types_{
    contact::ContactSectionName::Communication,
    contact::ContactSectionName::Profile};

const std::map<contact::ContactSectionName, int> Profile::sort_keys_{
    {contact::ContactSectionName::Communication, 0},
    {contact::ContactSectionName::Profile, 1}};

Profile::Profile(
    const api::client::Manager& api,
    const identifier::Nym& nymID,
    const SimpleCallback& cb) noexcept
    : ProfileList(api, nymID, cb, false)
    , listeners_({
          {api_.Endpoints().NymDownload(),
           new MessageProcessor<Profile>(&Profile::process_nym)},
      })
    , callbacks_()
    , name_(nym_name(api_.Wallet(), nymID))
    , payment_code_()
{
    setup_listeners(listeners_);
    startup_.reset(new std::thread(&Profile::startup, this));

    OT_ASSERT(startup_)
}

auto Profile::AddClaim(
    const contact::ContactSectionName section,
    const contact::ContactItemType type,
    const std::string& value,
    const bool primary,
    const bool active) const noexcept -> bool
{
    auto reason = api_.Factory().PasswordPrompt("Adding a claim to nym");
    auto nym = api_.Wallet().mutable_Nym(primary_id_, reason);

    switch (section) {
        case contact::ContactSectionName::Scope: {

            return nym.SetScope(type, value, primary, reason);
        }
        case contact::ContactSectionName::Communication: {
            switch (type) {
                case contact::ContactItemType::Email: {

                    return nym.AddEmail(value, primary, active, reason);
                }
                case contact::ContactItemType::Phone: {

                    return nym.AddPhoneNumber(value, primary, active, reason);
                }
                case contact::ContactItemType::Opentxs: {

                    return nym.AddPreferredOTServer(value, primary, reason);
                }
                default: {
                }
            }
        } break;
        case contact::ContactSectionName::Profile: {

            return nym.AddSocialMediaProfile(
                value, type, primary, active, reason);
        }
        case contact::ContactSectionName::Contract: {

            return nym.AddContract(value, type, primary, active, reason);
        }
        case contact::ContactSectionName::Procedure: {
            return nym.AddPaymentCode(value, type, primary, active, reason);
        }
        default: {
        }
    }

    Claim claim{};
    auto& [id, claimSection, claimType, claimValue, start, end, attributes] =
        claim;
    id = "";
    claimSection = contact::internal::translate(section);
    claimType = contact::internal::translate(type);
    claimValue = value;
    start = 0;
    end = 0;

    if (primary) {
        attributes.emplace(contact::internal::translate(
            contact::ContactItemAttribute::Primary));
    }

    if (primary || active) {
        attributes.emplace(contact::internal::translate(
            contact::ContactItemAttribute::Active));
    }

    return nym.AddClaim(claim, reason);
}

auto Profile::AllowedItems(
    const contact::ContactSectionName section,
    const std::string& lang) const noexcept -> Profile::ItemTypeList
{
    return ui::ProfileSection::AllowedItems(section, lang);
}

auto Profile::AllowedSections(const std::string& lang) const noexcept
    -> Profile::SectionTypeList
{
    SectionTypeList output{};

    for (const auto& type : allowed_types_) {
        output.emplace_back(
            type,
            proto::TranslateSectionName(
                contact::internal::translate(type), lang));
    }

    return output;
}

auto Profile::check_type(const contact::ContactSectionName type) noexcept
    -> bool
{
    return 1 == allowed_types_.count(type);
}

auto Profile::ClearCallbacks() const noexcept -> void
{
    Widget::ClearCallbacks();
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = {};
}

auto Profile::construct_row(
    const ProfileRowID& id,
    const ContactSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ProfileSectionWidget(*this, api_, id, index, custom);
}

auto Profile::Delete(
    const int sectionType,
    const int type,
    const std::string& claimID) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.Delete(type, claimID);
}

auto Profile::DisplayName() const noexcept -> std::string
{
    rLock lock{recursive_lock_};

    return name_;
}

auto Profile::nym_name(
    const api::Wallet& wallet,
    const identifier::Nym& nymID) noexcept -> std::string
{
    for (const auto& [id, name] : wallet.NymList()) {
        if (nymID.str() == id) { return name; }
    }

    return {};
}

auto Profile::PaymentCode() const noexcept -> std::string
{
    rLock lock{recursive_lock_};

    return payment_code_;
}

void Profile::process_nym(const identity::Nym& nym) noexcept
{
    {
        const auto name = nym.Alias();
        const auto code = nym.PaymentCode();
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

    auto active = std::set<ProfileRowID>{};

    for (const auto& section : nym.Claims()) {
        auto& type = section.first;

        if (check_type(type)) {
            CustomData custom{new opentxs::ContactSection(*section.second)};
            add_item(type, sort_key(type), custom);
            active.emplace(type);
        }
    }

    delete_inactive(active);
}

void Profile::process_nym(const Message& message) noexcept
{
    wait_for_startup();

    OT_ASSERT(1 < message.Body().size());

    auto nymID = api_.Factory().NymID();
    nymID->Assign(message.Body_at(1).Bytes());

    OT_ASSERT(false == nymID->empty())

    if (nymID != primary_id_) { return; }

    const auto nym = api_.Wallet().Nym(nymID);

    OT_ASSERT(nym)

    process_nym(*nym);
}

auto Profile::SetActive(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const bool active) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetActive(type, claimID, active);
}

auto Profile::SetCallbacks(Callbacks&& cb) noexcept -> void
{
    auto lock = Lock{callbacks_.lock_};
    callbacks_.cb_ = std::move(cb);
}

auto Profile::SetPrimary(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const bool primary) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetPrimary(type, claimID, primary);
}

auto Profile::SetValue(
    const int sectionType,
    const int type,
    const std::string& claimID,
    const std::string& value) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    auto& section = lookup(lock, static_cast<ProfileRowID>(sectionType));

    if (false == section.Valid()) { return false; }

    return section.SetValue(type, claimID, value);
}

auto Profile::sort_key(const contact::ContactSectionName type) noexcept -> int
{
    return sort_keys_.at(type);
}

void Profile::startup() noexcept
{
    LogVerbose(OT_METHOD)(__func__)(": Loading nym ")(primary_id_).Flush();
    const auto nym = api_.Wallet().Nym(primary_id_);

    OT_ASSERT(nym)

    process_nym(*nym);
    finish_startup();
}

Profile::~Profile()
{
    for (auto& it : listeners_) { delete it.second; }
}
}  // namespace opentxs::ui::implementation
