// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "identity/wot/verification/Group.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "internal/core/Factory.hpp"
#include "internal/identity/wot/verification/Verification.hpp"
#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/Verification.pb.h"
#include "serialization/protobuf/VerificationGroup.pb.h"
#include "serialization/protobuf/VerificationIdentity.pb.h"

namespace opentxs
{
auto Factory::VerificationGroup(
    identity::wot::verification::internal::Set& parent,
    const VersionNumber version,
    bool external) -> identity::wot::verification::internal::Group*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Group;

    try {

        return new ReturnType(parent, external, version);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            "Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::VerificationGroup(
    identity::wot::verification::internal::Set& parent,
    const proto::VerificationGroup& serialized,
    bool external) -> identity::wot::verification::internal::Group*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Group;

    try {

        return new ReturnType(parent, serialized, external);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            "Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}
}  // namespace opentxs

namespace opentxs::identity::wot::verification
{
const VersionNumber Group::DefaultVersion{1};
}

namespace opentxs::identity::wot::verification::implementation
{
Group::Group(
    internal::Set& parent,
    bool external,
    const VersionNumber version) noexcept
    : parent_(parent)
    , version_(version)
    , external_(external)
    , nyms_()
    , map_()
{
}

Group::Group(
    internal::Set& parent,
    const SerializedType& in,
    bool external) noexcept
    : parent_(parent)
    , version_(in.version())
    , external_(external)
    , nyms_(instantiate(*this, in))
    , map_()
{
}

Group::operator SerializedType() const noexcept
{
    auto output = SerializedType{};
    output.set_version(version_);

    for (const auto& pNym : nyms_) {
        OT_ASSERT(pNym);

        const auto& nym = *pNym;
        output.add_identity()->CopyFrom(nym);
    }

    return output;
}

auto Group::AddItem(
    const identifier::Nym& claimOwner,
    const Identifier& claim,
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    const Item::Type value,
    const Time start,
    const Time end,
    const VersionNumber version) noexcept -> bool
{
    if (external_) {
        LogError()(OT_PRETTY_CLASS())("Invalid internal item").Flush();

        return false;
    }

    return get_nym(claimOwner)
        .AddItem(claim, signer, reason, value, start, end, version);
}

auto Group::AddItem(
    const identifier::Nym& verifier,
    const Item::SerializedType verification) noexcept -> bool
{
    if (false == external_) {
        LogError()(OT_PRETTY_CLASS())("Invalid external item").Flush();

        return false;
    }

    if (verifier == parent_.NymID()) {
        LogError()(OT_PRETTY_CLASS())(
            "Attempting to add internal claim to external section")
            .Flush();

        return false;
    }

    return get_nym(verifier).AddItem(verification);
}

auto Group::DeleteItem(const Identifier& item) noexcept -> bool
{
    auto it = map_.find(item);

    if (map_.end() == it) { return false; }

    return get_nym(it->second).DeleteItem(item);
}

auto Group::get_nym(const identifier::Nym& id) noexcept -> internal::Nym&
{
    for (auto& pNym : nyms_) {
        OT_ASSERT(pNym);

        auto& nym = *pNym;

        if (id == nym.ID()) { return nym; }
    }

    auto pNym = std::unique_ptr<internal::Nym>{
        Factory::VerificationNym(*this, id, Nym::DefaultVersion)};

    OT_ASSERT(pNym);

    nyms_.emplace_back(std::move(pNym));

    return **nyms_.rbegin();
}

auto Group::instantiate(
    internal::Group& parent,
    const SerializedType& in) noexcept -> Vector
{
    auto output = Vector{};

    for (const auto& serialized : in.identity()) {
        auto pItem = std::unique_ptr<internal::Nym>{
            Factory::VerificationNym(parent, serialized)};

        if (pItem) { output.emplace_back(std::move(pItem)); }
    }

    return output;
}

auto Group::Register(const Identifier& id, const identifier::Nym& nym) noexcept
    -> void
{
    parent_.Register(id, external_);
    auto it = map_.find(id);

    if (map_.end() == it) {
        map_.emplace(id, nym);
    } else {
        it->second = nym;
    }
}

auto Group::Unregister(const Identifier& id) noexcept -> void
{
    parent_.Unregister(id);
    map_.erase(id);
}

auto Group::UpgradeNymVersion(const VersionNumber nymVersion) noexcept -> bool
{
    auto groupVersion{version_};

    try {
        while (true) {
            const auto [min, max] =
                proto::VerificationGroupAllowedIdentity().at(groupVersion);

            if (nymVersion < min) {
                LogError()(OT_PRETTY_CLASS())("Version ")(
                    nymVersion)(" too old")
                    .Flush();

                return false;
            }

            if (nymVersion > max) {
                ++groupVersion;
            } else {
                if (false == parent_.UpgradeGroupVersion(groupVersion)) {
                    return false;
                }

                return true;
            }
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("No support for version ")(
            nymVersion)(" items")
            .Flush();

        return false;
    }
}
}  // namespace opentxs::identity::wot::verification::implementation
