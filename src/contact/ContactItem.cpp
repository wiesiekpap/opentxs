// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/contact/ContactItem.hpp"  // IWYU pragma: associated

#include <memory>
#include <tuple>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/identity/credential/Contact.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"
#include "opentxs/protobuf/ContactItem.pb.h"

#define OT_METHOD "opentxs::ContactItem::"

namespace opentxs
{
ContactItem::ContactItem(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const proto::ContactSectionName section,
    const proto::ContactItemType& type,
    const std::string& value,
    const std::set<proto::ContactItemAttribute>& attributes,
    const std::time_t start,
    const std::time_t end,
    const std::string subtype)
    : api_(api)
    , version_(check_version(version, parentVersion))
    , nym_(nym)
    , section_(section)
    , type_(type)
    , value_(value)
    , start_(start)
    , end_(end)
    , attributes_(attributes)
    , id_(Identifier::Factory(
          identity::credential::Contact::
              ClaimID(api, nym, section, type, start, end, value, subtype)))
    , subtype_(subtype)
{
    if (0 == version) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: malformed version. "
                                           "Setting to ")(parentVersion)(".")
            .Flush();
    }
}

ContactItem::ContactItem(const ContactItem& rhs) noexcept
    : api_(rhs.api_)
    , version_(rhs.version_)
    , nym_(rhs.nym_)
    , section_(rhs.section_)
    , type_(rhs.type_)
    , value_(rhs.value_)
    , start_(rhs.start_)
    , end_(rhs.end_)
    , attributes_(rhs.attributes_)
    , id_(rhs.id_)
    , subtype_(rhs.subtype_)
{
}

ContactItem::ContactItem(ContactItem&& rhs) noexcept
    : api_(rhs.api_)
    , version_(rhs.version_)
    , nym_(std::move(const_cast<std::string&>(rhs.nym_)))
    , section_(rhs.section_)
    , type_(rhs.type_)
    , value_(std::move(const_cast<std::string&>(rhs.value_)))
    , start_(rhs.start_)
    , end_(rhs.end_)
    , attributes_(std::move(
          const_cast<std::set<proto::ContactItemAttribute>&>(rhs.attributes_)))
    , id_(std::move(const_cast<OTIdentifier&>(rhs.id_)))
    , subtype_(std::move(const_cast<std::string&>(rhs.subtype_)))
{
}

ContactItem::ContactItem(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const Claim& claim)
    : ContactItem(
          api,
          nym,
          version,
          parentVersion,
          static_cast<proto::ContactSectionName>(std::get<1>(claim)),
          static_cast<proto::ContactItemType>(std::get<2>(claim)),
          std::get<3>(claim),
          extract_attributes(claim),
          std::get<4>(claim),
          std::get<5>(claim),
          "")
{
}

ContactItem::ContactItem(
    const api::internal::Core& api,
    const std::string& nym,
    const VersionNumber parentVersion,
    const proto::ContactSectionName section,
    const proto::ContactItem& data)
    : ContactItem(
          api,
          nym,
          data.version(),
          parentVersion,
          section,
          data.type(),
          data.value(),
          extract_attributes(data),
          data.start(),
          data.end(),
          data.subtype())
{
}

auto ContactItem::operator==(const ContactItem& rhs) const -> bool
{
    if (false == (version_ == rhs.version_)) { return false; }

    if (false == (nym_ == rhs.nym_)) { return false; }

    if (false == (section_ == rhs.section_)) { return false; }

    if (false == (type_ == rhs.type_)) { return false; }

    if (false == (value_ == rhs.value_)) { return false; }

    if (false == (start_ == rhs.start_)) { return false; }

    if (false == (end_ == rhs.end_)) { return false; }

    if (false == (attributes_ == rhs.attributes_)) { return false; }

    if (false == (id_ == rhs.id_)) { return false; }

    return true;
}

ContactItem::operator proto::ContactItem() const { return Serialize(true); }

auto ContactItem::check_version(
    const VersionNumber in,
    const VersionNumber targetVersion) -> VersionNumber
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

auto ContactItem::End() const -> const std::time_t& { return end_; }

auto ContactItem::extract_attributes(const proto::ContactItem& serialized)
    -> std::set<proto::ContactItemAttribute>
{
    std::set<proto::ContactItemAttribute> output{};

    for (const auto& attribute : serialized.attribute()) {
        output.emplace(static_cast<proto::ContactItemAttribute>(attribute));
    }

    return output;
}

auto ContactItem::extract_attributes(const Claim& claim)
    -> std::set<proto::ContactItemAttribute>
{
    std::set<proto::ContactItemAttribute> output{};

    for (const auto& attribute : std::get<6>(claim)) {
        output.emplace(static_cast<proto::ContactItemAttribute>(attribute));
    }

    return output;
}

auto ContactItem::ID() const -> const Identifier& { return id_; }

auto ContactItem::isActive() const -> bool
{
    return 1 == attributes_.count(proto::CITEMATTR_ACTIVE);
}

auto ContactItem::isLocal() const -> bool
{
    return 1 == attributes_.count(proto::CITEMATTR_LOCAL);
}

auto ContactItem::isPrimary() const -> bool
{
    return 1 == attributes_.count(proto::CITEMATTR_PRIMARY);
}

auto ContactItem::Section() const -> const proto::ContactSectionName&
{
    return section_;
}

auto ContactItem::Serialize(const bool withID) const -> proto::ContactItem
{
    proto::ContactItem output{};
    output.set_version(version_);

    if (withID) { output.set_id(String::Factory(id_)->Get()); }

    output.set_type(type_);
    output.set_value(value_);
    output.set_start(start_);
    output.set_end(end_);

    for (const auto& attribute : attributes_) {
        output.add_attribute(attribute);
    }

    return output;
}

auto ContactItem::set_attribute(
    const proto::ContactItemAttribute& attribute,
    const bool value) const -> ContactItem
{
    const bool existingValue = 1 == attributes_.count(attribute);

    if (existingValue == value) { return *this; }

    auto attributes = attributes_;

    if (value) {
        attributes.emplace(attribute);

        if (proto::CITEMATTR_PRIMARY == attribute) {
            attributes.emplace(proto::CITEMATTR_ACTIVE);
        }
    } else {
        attributes.erase(attribute);
    }

    return ContactItem(
        api_,
        nym_,
        version_,
        version_,
        section_,
        type_,
        value_,
        attributes,
        start_,
        end_,
        subtype_);
}

auto ContactItem::SetActive(const bool active) const -> ContactItem
{
    return set_attribute(proto::CITEMATTR_ACTIVE, active);
}

auto ContactItem::SetEnd(const std::time_t end) const -> ContactItem
{
    if (end_ == end) { return *this; }

    return ContactItem(
        api_,
        nym_,
        version_,
        version_,
        section_,
        type_,
        value_,
        attributes_,
        start_,
        end,
        subtype_);
}

auto ContactItem::SetLocal(const bool local) const -> ContactItem
{
    return set_attribute(proto::CITEMATTR_LOCAL, local);
}

auto ContactItem::SetPrimary(const bool primary) const -> ContactItem
{
    return set_attribute(proto::CITEMATTR_PRIMARY, primary);
}

auto ContactItem::SetStart(const std::time_t start) const -> ContactItem
{
    if (start_ == start) { return *this; }

    return ContactItem(
        api_,
        nym_,
        version_,
        version_,
        section_,
        type_,
        value_,
        attributes_,
        start,
        end_,
        subtype_);
}

auto ContactItem::SetValue(const std::string& value) const -> ContactItem
{
    if (value_ == value) { return *this; }

    return ContactItem(
        api_,
        nym_,
        version_,
        version_,
        section_,
        type_,
        value,
        attributes_,
        start_,
        end_,
        subtype_);
}

auto ContactItem::Start() const -> const std::time_t& { return start_; }

auto ContactItem::Subtype() const -> const std::string& { return subtype_; }

auto ContactItem::Type() const -> const proto::ContactItemType&
{
    return type_;
}

auto ContactItem::Value() const -> const std::string& { return value_; }

auto ContactItem::Version() const -> VersionNumber { return version_; }
}  // namespace opentxs
