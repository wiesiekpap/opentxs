// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONTACT_CONTACTGROUP_HPP
#define OPENTXS_CONTACT_CONTACTGROUP_HPP

// IWYU pragma: no_include "opentxs/contact/ContactItemType.hpp"
// IWYU pragma: no_include "opentxs/contact/ContactSectionName.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace proto
{
class ContactSection;
}  // namespace proto

class ContactItem;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT ContactGroup
{
public:
    using ItemMap = std::map<OTIdentifier, std::shared_ptr<ContactItem>>;

    ContactGroup(
        const std::string& nym,
        const contact::ContactSectionName section,
        const contact::ContactItemType type,
        const ItemMap& items);
    ContactGroup(
        const std::string& nym,
        const contact::ContactSectionName section,
        const std::shared_ptr<ContactItem>& item);
    ContactGroup(const ContactGroup&) noexcept;
    ContactGroup(ContactGroup&&) noexcept;

    ContactGroup operator+(const ContactGroup& rhs) const;

    ItemMap::const_iterator begin() const;
    std::shared_ptr<ContactItem> Best() const;
    std::shared_ptr<ContactItem> Claim(const Identifier& item) const;
    bool HaveClaim(const Identifier& item) const;
    ContactGroup AddItem(const std::shared_ptr<ContactItem>& item) const;
    ContactGroup AddPrimary(const std::shared_ptr<ContactItem>& item) const;
    ContactGroup Delete(const Identifier& id) const;
    ItemMap::const_iterator end() const;
    const Identifier& Primary() const;
    std::shared_ptr<ContactItem> PrimaryClaim() const;
    bool SerializeTo(proto::ContactSection& section, const bool withIDs = false)
        const;
    std::size_t Size() const;
    const contact::ContactItemType& Type() const;

    ~ContactGroup();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactGroup() = delete;
    ContactGroup& operator=(const ContactGroup&) = delete;
    ContactGroup& operator=(ContactGroup&&) = delete;
};
}  // namespace opentxs
#endif
