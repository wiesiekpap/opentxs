// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACTSECTION_HPP
#define OPENTXS_CONTACT_CONTACTSECTION_HPP

// IWYU pragma: no_include "opentxs/contact/ContactSectionName.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"

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
class ContactSection;
}  // namespace proto

class ContactGroup;
class ContactItem;
class Identifier;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT ContactSection
{
public:
    using GroupMap =
        std::map<contact::ContactItemType, std::shared_ptr<ContactGroup>>;

    ContactSection(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const contact::ContactSectionName section,
        const GroupMap& groups);
    ContactSection(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const contact::ContactSectionName section,
        const std::shared_ptr<ContactItem>& item);
    OPENTXS_NO_EXPORT ContactSection(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const proto::ContactSection& serialized);
    ContactSection(
        const api::internal::Core& api,
        const std::string& nym,
        const VersionNumber parentVersion,
        const ReadView& serialized);
    ContactSection(const ContactSection&) noexcept;
    ContactSection(ContactSection&&) noexcept;

    ContactSection operator+(const ContactSection& rhs) const;

    ContactSection AddItem(const std::shared_ptr<ContactItem>& item) const;
    GroupMap::const_iterator begin() const;
    std::shared_ptr<ContactItem> Claim(const Identifier& item) const;
    ContactSection Delete(const Identifier& id) const;
    GroupMap::const_iterator end() const;
    std::shared_ptr<ContactGroup> Group(
        const contact::ContactItemType& type) const;
    bool HaveClaim(const Identifier& item) const;
    bool Serialize(AllocateOutput destination, const bool withIDs = false)
        const;
    OPENTXS_NO_EXPORT bool SerializeTo(
        proto::ContactData& data,
        const bool withIDs = false) const;
    std::size_t Size() const;
    const contact::ContactSectionName& Type() const;
    VersionNumber Version() const;

    ~ContactSection();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactSection() = delete;
    ContactSection& operator=(const ContactSection&) = delete;
    ContactSection& operator=(ContactSection&&) = delete;
};
}  // namespace opentxs
#endif
