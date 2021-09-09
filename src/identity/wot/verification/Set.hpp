// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>
#include <memory>

#include "internal/identity/wot/verification/Verification.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/verification/Group.hpp"
#include "opentxs/identity/wot/verification/Item.hpp"
#include "opentxs/identity/wot/verification/Set.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

class Factory;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::identity::wot::verification::implementation
{
class Set final : public internal::Set
{
public:
    operator SerializedType() const noexcept final;

    auto API() const noexcept -> const api::Core& final { return api_; }
    auto External() const noexcept -> const Group& final { return *external_; }
    auto Internal() const noexcept -> const Group& final { return *internal_; }
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return nym_id_;
    }
    auto Version() const noexcept -> VersionNumber final { return version_; }

    auto AddItem(
        const identifier::Nym& claimOwner,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value,
        const Time start,
        const Time end,
        const VersionNumber version) noexcept -> bool final;
    auto AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept -> bool final;
    auto DeleteItem(const Identifier& item) noexcept -> bool final;
    auto External() noexcept -> Group& final { return *external_; }
    auto Internal() noexcept -> Group& final { return *internal_; }
    void Register(const Identifier& id, const bool external) noexcept final;
    void Unregister(const Identifier& id) noexcept final;
    auto UpgradeGroupVersion(const VersionNumber groupVersion) noexcept
        -> bool final;

    ~Set() final = default;

private:
    friend opentxs::Factory;

    using GroupPointer = std::unique_ptr<internal::Group>;
    using ChildType = verification::Group::SerializedType;

    const api::Core& api_;
    const VersionNumber version_;
    const OTNymID nym_id_;
    GroupPointer internal_;
    GroupPointer external_;
    std::map<OTIdentifier, bool> map_;

    static auto instantiate(
        internal::Set& parent,
        const ChildType& serialized,
        bool external) noexcept -> GroupPointer;

    Set(const api::Core& api,
        const identifier::Nym& nym,
        const VersionNumber version = DefaultVersion) noexcept(false);
    Set(const api::Core& api,
        const identifier::Nym& nym,
        const SerializedType& serialized) noexcept(false);
    Set() = delete;
    Set(const Set&) = delete;
    Set(Set&&) = delete;
    auto operator=(const Set&) -> Set& = delete;
    auto operator=(Set&&) -> Set& = delete;
};
}  // namespace opentxs::identity::wot::verification::implementation
