// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_WOT_VERIFICATION_ITEM_HPP
#define OPENTXS_IDENTITY_WOT_VERIFICATION_ITEM_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace proto
{
class Signature;
class Verification;
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
class Item
{
public:
    using SerializedType = proto::Verification;

    enum class Type : bool { Confirm = true, Refute = false };
    enum class Validity : bool { Active = false, Retracted = true };

    OPENTXS_EXPORT static const VersionNumber DefaultVersion;

    OPENTXS_EXPORT virtual operator SerializedType() const noexcept = 0;

    OPENTXS_EXPORT virtual Time Begin() const noexcept = 0;
    OPENTXS_EXPORT virtual const Identifier& ClaimID() const noexcept = 0;
    OPENTXS_EXPORT virtual Time End() const noexcept = 0;
    OPENTXS_EXPORT virtual const Identifier& ID() const noexcept = 0;
    OPENTXS_EXPORT virtual const proto::Signature& Signature()
        const noexcept = 0;
    OPENTXS_EXPORT virtual Validity Valid() const noexcept = 0;
    OPENTXS_EXPORT virtual Type Value() const noexcept = 0;
    OPENTXS_EXPORT virtual VersionNumber Version() const noexcept = 0;

    OPENTXS_EXPORT virtual ~Item() = default;

protected:
    Item() = default;

private:
    Item(const Item&) = delete;
    Item(Item&&) = delete;
    Item& operator=(const Item&) = delete;
    Item& operator=(Item&&) = delete;
};
}  // namespace verification
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
#endif
