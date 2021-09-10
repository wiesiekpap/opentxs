// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_NYM_HPP
#define OPENTXS_CORE_IDENTIFIER_NYM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

using OTNymID = Pimpl<identifier::Nym>;

OPENTXS_EXPORT auto operator==(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs
{
namespace identifier
{
class OPENTXS_EXPORT Nym : virtual public opentxs::Identifier
{
public:
    static auto Factory() -> OTNymID;
    static auto Factory(const std::string& rhs) -> OTNymID;
    static auto Factory(const String& rhs) -> OTNymID;
    static auto Factory(const identity::Nym& nym) -> OTNymID;

    ~Nym() override = default;

protected:
    Nym() = default;

private:
    friend OTNymID;

#ifndef _WIN32
    auto clone() const -> Nym* override = 0;
#endif
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    auto operator=(const Nym&) -> Nym& = delete;
    auto operator=(Nym&&) -> Nym& = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
