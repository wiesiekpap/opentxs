// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_WOT_VERIFICATION_ITEM_HPP
#define OPENTXS_IDENTITY_WOT_VERIFICATION_ITEM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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
class OPENTXS_EXPORT Item
{
public:
    using SerializedType = proto::Verification;

    enum class Type : bool { Confirm = true, Refute = false };
    enum class Validity : bool { Active = false, Retracted = true };

    static const VersionNumber DefaultVersion;

    OPENTXS_NO_EXPORT virtual operator SerializedType() const noexcept = 0;

    virtual auto Begin() const noexcept -> Time = 0;
    virtual auto ClaimID() const noexcept -> const Identifier& = 0;
    virtual auto End() const noexcept -> Time = 0;
    virtual auto ID() const noexcept -> const Identifier& = 0;
    virtual auto Signature() const noexcept -> const proto::Signature& = 0;
    virtual auto Valid() const noexcept -> Validity = 0;
    virtual auto Value() const noexcept -> Type = 0;
    virtual auto Version() const noexcept -> VersionNumber = 0;

    virtual ~Item() = default;

protected:
    Item() = default;

private:
    Item(const Item&) = delete;
    Item(Item&&) = delete;
    auto operator=(const Item&) -> Item& = delete;
    auto operator=(Item&&) -> Item& = delete;
};
}  // namespace verification
}  // namespace wot
}  // namespace identity
}  // namespace opentxs
#endif
