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
namespace internal
{
struct Core;
}  // namespace internal
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

    OPENTXS_NO_EXPORT static std::string PrintContactData(
        const proto::ContactData& data);

    ContactData(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber targetVersion,
        const SectionMap& sections);
    OPENTXS_NO_EXPORT ContactData(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber targetVersion,
        const proto::ContactData& serialized);
    ContactData(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber targetVersion,
        const ReadView& serialized);
    ContactData(const ContactData&);

    ContactData operator+(const ContactData& rhs) const;

    operator std::string() const;

    ContactData AddContract(
        const std::string& instrumentDefinitionID,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active) const;
    ContactData AddEmail(
        const std::string& value,
        const bool primary,
        const bool active) const;
    ContactData AddItem(const Claim& claim) const;
    ContactData AddItem(const std::shared_ptr<ContactItem>& item) const;
    ContactData AddPaymentCode(
        const std::string& code,
        const contact::ContactItemType currency,
        const bool primary,
        const bool active) const;
    ContactData AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active) const;
    ContactData AddPreferredOTServer(const Identifier& id, const bool primary)
        const;
    ContactData AddSocialMediaProfile(
        const std::string& value,
        const contact::ContactItemType type,
        const bool primary,
        const bool active) const;
    SectionMap::const_iterator begin() const;
    std::string BestEmail() const;
    std::string BestPhoneNumber() const;
    std::string BestSocialMediaProfile(
        const contact::ContactItemType type) const;
    std::shared_ptr<ContactItem> Claim(const Identifier& item) const;
    std::set<OTIdentifier> Contracts(
        const contact::ContactItemType currency,
        const bool onlyActive) const;
    ContactData Delete(const Identifier& id) const;
    std::string EmailAddresses(bool active = true) const;
    SectionMap::const_iterator end() const;
    std::shared_ptr<ContactGroup> Group(
        const contact::ContactSectionName& section,
        const contact::ContactItemType& type) const;
    bool HaveClaim(const Identifier& item) const;
    bool HaveClaim(
        const contact::ContactSectionName& section,
        const contact::ContactItemType& type,
        const std::string& value) const;
    std::string Name() const;
    std::string PhoneNumbers(bool active = true) const;
    OTServerID PreferredOTServer() const;
    std::shared_ptr<ContactSection> Section(
        const contact::ContactSectionName& section) const;
    bool Serialize(AllocateOutput destination, const bool withID = false) const;
    OPENTXS_NO_EXPORT bool Serialize(
        proto::ContactData& out,
        const bool withID = false) const;
    ContactData SetCommonName(const std::string& name) const;
    ContactData SetName(const std::string& name, const bool primary = true)
        const;
    ContactData SetScope(
        const contact::ContactItemType type,
        const std::string& name) const;
    std::string SocialMediaProfiles(
        const contact::ContactItemType type,
        bool active = true) const;
    const std::set<contact::ContactItemType> SocialMediaProfileTypes() const;
    contact::ContactItemType Type() const;
    VersionNumber Version() const;

    ~ContactData();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactData() = delete;
    ContactData(ContactData&&) = delete;
    ContactData& operator=(const ContactData&) = delete;
    ContactData& operator=(ContactData&&) = delete;
};
}  // namespace opentxs
#endif
