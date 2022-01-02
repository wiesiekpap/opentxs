// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "core/ui/base/List.hpp"
#include "core/ui/base/Widget.hpp"
#include "internal/core/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/ui/Profile.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
class Wallet;
}  // namespace session
}  // namespace api

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ProfileList = List<
    ProfileExternalInterface,
    ProfileInternalInterface,
    ProfileRowID,
    ProfileRowInterface,
    ProfileRowInternal,
    ProfileRowBlank,
    ProfileSortKey,
    ProfilePrimaryID>;

class Profile final : public ProfileList
{
public:
    auto AddClaim(
        const identity::wot::claim::SectionType section,
        const identity::wot::claim::ClaimType type,
        const std::string& value,
        const bool primary,
        const bool active) const noexcept -> bool final;
    auto AllowedItems(
        const identity::wot::claim::SectionType section,
        const std::string& lang) const noexcept -> ItemTypeList final;
    auto AllowedSections(const std::string& lang) const noexcept
        -> SectionTypeList final;
    auto ClearCallbacks() const noexcept -> void final;
    auto Delete(const int section, const int type, const std::string& claimID)
        const noexcept -> bool final;
    auto DisplayName() const noexcept -> std::string final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto ID() const noexcept -> std::string final { return primary_id_->str(); }
    auto PaymentCode() const noexcept -> std::string final;
    auto SetActive(
        const int section,
        const int type,
        const std::string& claimID,
        const bool active) const noexcept -> bool final;
    auto SetPrimary(
        const int section,
        const int type,
        const std::string& claimID,
        const bool primary) const noexcept -> bool final;
    auto SetValue(
        const int section,
        const int type,
        const std::string& claimID,
        const std::string& value) const noexcept -> bool final;

    auto SetCallbacks(Callbacks&& cb) noexcept -> void final;

    Profile(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;
    ~Profile() final;

private:
    struct CallbackHolder {
        mutable std::mutex lock_{};
        Callbacks cb_{};
    };

    const ListenerDefinitions listeners_;
    mutable CallbackHolder callbacks_;
    std::string name_;
    std::string payment_code_;

    static const std::set<identity::wot::claim::SectionType> allowed_types_;
    static const std::map<identity::wot::claim::SectionType, int> sort_keys_;

    static auto sort_key(const identity::wot::claim::SectionType type) noexcept
        -> int;
    static auto check_type(
        const identity::wot::claim::SectionType type) noexcept -> bool;
    static auto nym_name(
        const api::session::Wallet& wallet,
        const identifier::Nym& nymID) noexcept -> std::string;

    auto construct_row(
        const ProfileRowID& id,
        const ProfileSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ProfileRowID& id) const noexcept -> bool final
    {
        return ProfileList::last(id);
    }

    auto process_nym(const identity::Nym& nym) noexcept -> void;
    auto process_nym(const Message& message) noexcept -> void;
    auto startup() noexcept -> void;

    Profile() = delete;
    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    auto operator=(const Profile&) -> Profile& = delete;
    auto operator=(Profile&&) -> Profile& = delete;
};
}  // namespace opentxs::ui::implementation
