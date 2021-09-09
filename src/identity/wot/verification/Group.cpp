// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "identity/wot/verification/Group.hpp"  // IWYU pragma: associated

#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "2_Factory.hpp"
#include "internal/identity/wot/verification/Verification.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Nym.hpp"
#include "opentxs/protobuf/Basic.hpp"
#include "opentxs/protobuf/VerificationGroup.pb.h"
#include "opentxs/protobuf/VerificationIdentity.pb.h"
#include "opentxs/protobuf/verify/VerifyContacts.hpp"

#define OT_METHOD                                                              \
    "opentxs::identity::wot::verification::implementation::Group::"

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
        LogOutput("opentxs::Factory::")(__func__)(
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
        LogOutput("opentxs::Factory::")(__func__)(
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
        LogOutput(OT_METHOD)(__func__)(": Invalid internal item").Flush();

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
        LogOutput(OT_METHOD)(__func__)(": Invalid external item").Flush();

        return false;
    }

    if (verifier == parent_.NymID()) {
        LogOutput(OT_METHOD)(__func__)(
            ": Attempting to add internal claim to external section")
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
                LogOutput(OT_METHOD)(__func__)(": Version ")(
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
        LogOutput(OT_METHOD)(__func__)(": No support for version ")(
            nymVersion)(" items")
            .Flush();

        return false;
    }
}
}  // namespace opentxs::identity::wot::verification::implementation
