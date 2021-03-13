// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "opentxs/contact/ContactGroup.hpp"  // IWYU pragma: associated

#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/ContactItem.pb.h"
#include "opentxs/protobuf/ContactSection.pb.h"

#define OT_METHOD "opentxs::ContactGroup::"

namespace opentxs
{
ContactGroup::ContactGroup(
    const std::string& nym,
    const proto::ContactSectionName section,
    const proto::ContactItemType type,
    const ItemMap& items)
    : nym_(nym)
    , section_(section)
    , type_(type)
    , primary_(Identifier::Factory(get_primary_item(items)))
    , items_(normalize_items(items))
{
    for (const auto& it : items_) { OT_ASSERT(it.second); }
}

ContactGroup::ContactGroup(
    const std::string& nym,
    const proto::ContactSectionName section,
    const std::shared_ptr<ContactItem>& item)
    : ContactGroup(nym, section, item->Type(), create_item(item))
{
    OT_ASSERT(item);
}

auto ContactGroup::operator+(const ContactGroup& rhs) const -> ContactGroup
{
    OT_ASSERT(section_ == rhs.section_);

    auto primary = Identifier::Factory();

    if (primary_->empty()) { primary = Identifier::Factory(rhs.primary_); }

    auto map{items_};

    for (const auto& it : rhs.items_) {
        const auto& item = it.second;

        OT_ASSERT(item);
        const auto& id = item->ID();
        const bool exists = (1 == map.count(id));

        if (exists) { continue; }

        const bool isPrimary = item->isPrimary();
        const bool designated = (id == primary);

        if (isPrimary && (false == designated)) {
            map.emplace(id, new ContactItem(item->SetPrimary(false)));
        } else {
            map.emplace(id, item);
        }

        OT_ASSERT(map[id])
    }

    return ContactGroup(nym_, section_, type_, map);
}

auto ContactGroup::AddItem(const std::shared_ptr<ContactItem>& item) const
    -> ContactGroup
{
    OT_ASSERT(item);

    if (item->isPrimary()) { return AddPrimary(item); }

    const auto& id = item->ID();
    const bool alreadyExists =
        (1 == items_.count(id)) && (*item == *items_.at(id));

    if (alreadyExists) { return *this; }

    auto map = items_;
    map[id] = item;

    return ContactGroup(nym_, section_, type_, map);
}

auto ContactGroup::AddPrimary(const std::shared_ptr<ContactItem>& item) const
    -> ContactGroup
{
    if (false == bool(item)) { return *this; }

    const auto& incomingID = item->ID();
    const bool isExistingPrimary = (primary_ == incomingID);
    const bool haveExistingPrimary =
        ((false == primary_->empty()) && (false == isExistingPrimary));
    auto map = items_;
    auto& newPrimary = map[incomingID];
    newPrimary.reset(new ContactItem(item->SetPrimary(true)));

    OT_ASSERT(newPrimary);

    if (haveExistingPrimary) {
        auto& oldPrimary = map.at(primary_);

        OT_ASSERT(oldPrimary);

        oldPrimary.reset(new ContactItem(oldPrimary->SetPrimary(false)));

        OT_ASSERT(oldPrimary);
    }

    return ContactGroup(nym_, section_, type_, map);
}

auto ContactGroup::begin() const -> ContactGroup::ItemMap::const_iterator
{
    return items_.cbegin();
}

auto ContactGroup::Best() const -> std::shared_ptr<ContactItem>
{
    if (0 == items_.size()) { return {}; }

    if (false == primary_->empty()) { return items_.at(primary_); }

    for (const auto& it : items_) {
        const auto& claim = it.second;

        OT_ASSERT(claim);

        if (claim->isActive()) { return claim; }
    }

    return items_.begin()->second;
}

auto ContactGroup::Claim(const Identifier& item) const
    -> std::shared_ptr<ContactItem>
{
    auto it = items_.find(item);

    if (items_.end() == it) { return {}; }

    return it->second;
}

auto ContactGroup::create_item(const std::shared_ptr<ContactItem>& item)
    -> ContactGroup::ItemMap
{
    OT_ASSERT(item);

    ItemMap output{};
    output[item->ID()] = item;

    return output;
}

auto ContactGroup::Delete(const Identifier& id) const -> ContactGroup
{
    const bool exists = (1 == items_.count(id));

    if (false == exists) { return *this; }

    auto map = items_;
    map.erase(id);

    return ContactGroup(nym_, section_, type_, map);
}

auto ContactGroup::end() const -> ContactGroup::ItemMap::const_iterator
{
    return items_.cend();
}

auto ContactGroup::get_primary_item(const ItemMap& items) -> OTIdentifier
{
    auto primary = Identifier::Factory();

    for (const auto& it : items) {
        const auto& item = it.second;

        OT_ASSERT(item);

        if (item->isPrimary()) {
            primary = item->ID();

            break;
        }
    }

    return primary;
}

auto ContactGroup::normalize_items(const ItemMap& items)
    -> ContactGroup::ItemMap
{
    auto primary = Identifier::Factory();

    auto map = items;

    for (const auto& it : map) {
        const auto& item = it.second;

        OT_ASSERT(item);

        if (item->isPrimary()) {
            if (primary->empty()) {
                primary = item->ID();
            } else {
                const auto& id = item->ID();
                map[id].reset(new ContactItem(item->SetPrimary(false)));
            }
        }
    }

    return map;
}

auto ContactGroup::HaveClaim(const Identifier& item) const -> bool
{
    return (1 == items_.count(item));
}

auto ContactGroup::Primary() const -> const Identifier& { return primary_; }

auto ContactGroup::SerializeTo(
    proto::ContactSection& section,
    const bool withIDs) const -> bool
{
    if (section.name() != section_) {

        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Trying to serialize to incorrect section.")
            .Flush();

        return false;
    }

    for (const auto& it : items_) {
        const auto& item = it.second;

        OT_ASSERT(item);

        *section.add_item() = item->Serialize(withIDs);
    }

    return true;
}

auto ContactGroup::PrimaryClaim() const -> std::shared_ptr<ContactItem>
{
    if (primary_->empty()) { return {}; }

    return items_.at(primary_);
}

auto ContactGroup::Size() const -> std::size_t { return items_.size(); }

auto ContactGroup::Type() const -> const proto::ContactItemType&
{
    return type_;
}
}  // namespace opentxs
