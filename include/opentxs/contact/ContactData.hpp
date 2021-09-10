// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACTDATA_HPP
#define OPENTXS_CONTACT_CONTACTDATA_HPP

// IWYU pragma: no_include "opentxs/contact/ContactItemType.hpp"
// IWYU pragma: no_include "opentxs/contact/ContactSectionName.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Server.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace proto
{
class ContactData;
}  // namespace proto

class ContactGroup;
class ContactItem;
class ContactSection;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT ContactData
{
public:
    using SectionMap =
        std::map<contact::ContactSectionName, std::shared_ptr<ContactSection>>;

    OPENTXS_NO_EXPORT static auto PrintContactData(
        const proto::ContactData& data) -> std::string;

    ContactData(
        const api::Core& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber targetVersion,
        const SectionMap& sections);
    OPENTXS_NO_EXPORT ContactData(
        const api::Core& api,
        const std::string& nym,
        const VersionNumber targetVersion,
        const proto::ContactData& serialized);
    ContactData(
        const api::Core& api,
        const std::string& nym,
        const VersionNumber targetVersion,
        const ReadView& serialized);
    ContactData(const ContactData&);

    auto operator+(const ContactData& rhs) const -> ContactData;

    operator std::string() const;

    auto AddContract(
        const std::string& instrumentDefinitionID,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddEmail(
        const std::string& value,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddItem(const Claim& claim) const -> ContactData;
    auto AddItem(const std::shared_ptr<ContactItem>& item) const -> ContactData;
    auto AddPaymentCode(
        const std::string& code,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddPreferredOTServer(const Identifier& id, const bool primary) const
        -> ContactData;
    auto AddSocialMediaProfile(
        const std::string& value,
        const contact::ContactItemType type,
        const bool primary,
        const bool active) const -> ContactData;
    auto begin() const -> SectionMap::const_iterator;
    auto BestEmail() const -> std::string;
    auto BestPhoneNumber() const -> std::string;
    auto BestSocialMediaProfile(const contact::ContactItemType type) const
        -> std::string;
    auto Claim(const Identifier& item) const -> std::shared_ptr<ContactItem>;
    auto Contracts(
        const contact::ContactItemType currency,
        const bool onlyActive) const -> std::set<OTIdentifier>;
    auto Delete(const Identifier& id) const -> ContactData;
    auto EmailAddresses(bool active = true) const -> std::string;
    auto end() const -> SectionMap::const_iterator;
    auto Group(
        const contact::ContactSectionName& section,
        const contact::ContactItemType& type) const
        -> std::shared_ptr<ContactGroup>;
    auto HaveClaim(const Identifier& item) const -> bool;
    auto HaveClaim(
        const contact::ContactSectionName& section,
        const contact::ContactItemType& type,
        const std::string& value) const -> bool;
    auto Name() const -> std::string;
    auto PhoneNumbers(bool active = true) const -> std::string;
    auto PreferredOTServer() const -> OTServerID;
    auto Section(const contact::ContactSectionName& section) const
        -> std::shared_ptr<ContactSection>;
    auto Serialize(AllocateOutput destination, const bool withID = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::ContactData& out,
        const bool withID = false) const -> bool;
    auto SetCommonName(const std::string& name) const -> ContactData;
    auto SetName(const std::string& name, const bool primary = true) const
        -> ContactData;
    auto SetScope(const contact::ContactItemType type, const std::string& name)
        const -> ContactData;
    auto SocialMediaProfiles(
        const contact::ContactItemType type,
        bool active = true) const -> std::string;
    auto SocialMediaProfileTypes() const
        -> const std::set<contact::ContactItemType>;
    auto Type() const -> contact::ContactItemType;
    auto Version() const -> VersionNumber;

    ~ContactData();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactData() = delete;
    ContactData(ContactData&&) = delete;
    auto operator=(const ContactData&) -> ContactData& = delete;
    auto operator=(ContactData&&) -> ContactData& = delete;
};
}  // namespace opentxs
#endif
