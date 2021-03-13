// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_WOT_VERIFICATION_SET_HPP
#define OPENTXS_IDENTITY_WOT_VERIFICATION_SET_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace proto
{
class VerificationSet;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace identity
{
namespace wot
{
namespace verification
{
class Set
{
public:
    using SerializedType = proto::VerificationSet;

    OPENTXS_EXPORT static const VersionNumber DefaultVersion;

    OPENTXS_EXPORT virtual operator SerializedType() const noexcept = 0;

    OPENTXS_EXPORT virtual const Group& External() const noexcept = 0;
    OPENTXS_EXPORT virtual const Group& Internal() const noexcept = 0;
    OPENTXS_EXPORT virtual VersionNumber Version() const noexcept = 0;

    OPENTXS_EXPORT virtual bool AddItem(
        const identifier::Nym& claimOwner,
        const Identifier& claim,
        const identity::Nym& signer,
        const PasswordPrompt& reason,
        const Item::Type value = Item::Type::Confirm,
        const Time start = {},
        const Time end = {},
        const VersionNumber version = Item::DefaultVersion) noexcept = 0;
    OPENTXS_EXPORT virtual bool AddItem(
        const identifier::Nym& verifier,
        const Item::SerializedType verification) noexcept = 0;
    OPENTXS_EXPORT virtual bool DeleteItem(const Identifier& item) noexcept = 0;
    OPENTXS_EXPORT virtual Group& External() noexcept = 0;
    OPENTXS_EXPORT virtual Group& Internal() noexcept = 0;

    OPENTXS_EXPORT virtual ~Set() = default;

protected:
    Set() = default;

private:
    Set(const Set&) = delete;
    Set(Set&&) = delete;
    Set& operator=(const Set&) = delete;
    Set& operator=(Set&&) = delete;
};
}  // namespace verification
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
#endif
