// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "identity/wot/verification/Nym.hpp"  // IWYU pragma: associated

#include <chrono>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Proto.hpp"
#include "internal/core/Factory.hpp"
#include "internal/identity/wot/verification/Verification.hpp"
#include "internal/serialization/protobuf/Basic.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/Verification.pb.h"
#include "serialization/protobuf/VerificationIdentity.pb.h"

namespace opentxs
{
auto Factory::VerificationNym(
    identity::wot::verification::internal::Group& parent,
    const identifier::Nym& nym,
    const VersionNumber version) -> identity::wot::verification::internal::Nym*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Nym;

    try {

        return new ReturnType(parent, nym, version);
    } catch (const std::exception& e) {
        LogError()("opentxs::Factory::")(__func__)(
            "Failed to construct verification nym: ")(e.what())
            .Flush();

        return nullptr;
    }
}

auto Factory::VerificationNym(
    identity::wot::verification::internal::Group& parent,
    const proto::VerificationIdentity& serialized)
    -> identity::wot::verification::internal::Nym*
{
    using ReturnType =
        opentxs::identity::wot::verification::implementation::Nym;

    try {

        return new ReturnType(parent, serialized);
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
const VersionNumber Nym::DefaultVersion{1};
}

namespace opentxs::identity::wot::verification::implementation
{
Nym::Nym(
    internal::Group& parent,
    const identifier::Nym& nym,
    const VersionNumber version) noexcept
    : parent_(parent)
    , version_(version)
    , id_(nym)
    , items_()
{
}

Nym::Nym(internal::Group& parent, const SerializedType& in) noexcept
    : parent_(parent)
    , version_(in.version())
    , id_(parent_.API().Factory().NymID(in.nym()))
    , items_(instantiate(*this, in))
{
}

Nym::operator SerializedType() const noexcept
{
    auto output = SerializedType{};
    output.set_version(version_);
    output.set_nym(id_->str());

    for (const auto& pItem : items_) {
        OT_ASSERT(pItem);

        const auto& item = *pItem;
        output.add_verification()->CopyFrom(item);
    }

    return output;
}

auto Nym::AddItem(
    const Identifier& claim,
    const identity::Nym& signer,
    const PasswordPrompt& reason,
    const Item::Type value,
    const Time start,
    const Time end,
    const VersionNumber version) noexcept -> bool
{
    auto pCandidate = Child{Factory::VerificationItem(
        *this,
        claim,
        signer,
        reason,
        static_cast<bool>(value),
        start,
        end,
        version)};

    if (false == bool(pCandidate)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct item").Flush();

        return false;
    }

    return add_item(std::move(pCandidate));
}

auto Nym::AddItem(const Item::SerializedType item) noexcept -> bool
{
    auto pCandidate = Child{Factory::VerificationItem(*this, item)};

    if (false == bool(pCandidate)) {
        LogError()(OT_PRETTY_CLASS())("Failed to construct item").Flush();

        return false;
    }

    return add_item(std::move(pCandidate));
}

auto Nym::add_item(Child pCandidate) noexcept -> bool
{
    OT_ASSERT(pCandidate)

    auto accept{true};
    const auto& candidate = *pCandidate;

    auto nymVersion{version_};

    if (false == UpgradeItemVersion(candidate.Version(), nymVersion)) {
        return false;
    }

    if (nymVersion != version_) {
        if (false == parent_.UpgradeNymVersion(nymVersion)) { return false; }

        const_cast<VersionNumber&>(version_) = nymVersion;
    }

    for (auto i{items_.cbegin()}; i != items_.cend();) {
        auto& pItem = *i;

        OT_ASSERT(pItem);

        const auto& item = *pItem;

        switch (match(candidate, item)) {
            case Match::Replace: {
                i = items_.erase(i);
            } break;
            case Match::Reject: {
                accept = false;
                [[fallthrough]];
            }
            case Match::Accept:
            default: {
                ++i;
            }
        }
    }

    if (accept) {
        parent_.Register(candidate.ID(), id_);
        items_.emplace_back(std::move(pCandidate));
    }

    return accept;
}

auto Nym::DeleteItem(const Identifier& id) noexcept -> bool
{
    auto output{false};

    for (auto i{items_.cbegin()}; i != items_.cend(); ++i) {
        auto& pItem = *i;

        OT_ASSERT(pItem);

        const auto& item = *pItem;

        if (item.ID() == id) {
            output = true;
            items_.erase(i);
            parent_.Unregister(id);
            break;
        }
    }

    return output;
}

auto Nym::instantiate(internal::Nym& parent, const SerializedType& in) noexcept
    -> Vector
{
    auto output = Vector{};

    for (const auto& serialized : in.verification()) {
        auto pItem = std::unique_ptr<internal::Item>{
            Factory::VerificationItem(parent, serialized)};

        if (pItem) { output.emplace_back(std::move(pItem)); }
    }

    return output;
}

auto Nym::match(const internal::Item& lhs, const internal::Item& rhs) noexcept
    -> Match
{
    if (lhs.ClaimID() != rhs.ClaimID()) { return Match::Accept; }

    const auto subset =
        (lhs.Begin() >= rhs.Begin()) && (lhs.End() <= rhs.End());
    const auto superset =
        (lhs.Begin() <= rhs.Begin()) && (lhs.End() >= rhs.End());
    const auto overlapBegin =
        (lhs.Begin() >= rhs.Begin()) && (lhs.Begin() <= rhs.End());
    const auto overlapEnd =
        (lhs.End() >= rhs.Begin()) && (lhs.End() <= rhs.End());
    const auto overlap = overlapBegin || overlapEnd;

    if (subset) { return Match::Reject; }

    if (superset) { return Match::Replace; }

    if (false == overlap) { return Match::Accept; }

    if (lhs.Value() != rhs.Value()) { return Match::Reject; }

    if (lhs.Valid() != rhs.Valid()) { return Match::Reject; }

    return Match::Accept;
}

auto Nym::UpgradeItemVersion(
    const VersionNumber itemVersion,
    VersionNumber& nymVersion) noexcept -> bool
{
    try {
        while (true) {
            const auto [min, max] =
                proto::VerificationIdentityAllowedVerification().at(nymVersion);

            if (itemVersion < min) {
                LogError()(OT_PRETTY_CLASS())("Version ")(
                    itemVersion)(" too old")
                    .Flush();

                return false;
            }

            if (itemVersion > max) {
                ++nymVersion;
            } else {

                return true;
            }
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("No support for version ")(
            itemVersion)(" items")
            .Flush();

        return false;
    }
}
}  // namespace opentxs::identity::wot::verification::implementation
