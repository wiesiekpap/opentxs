// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_UNITDEFINITION_HPP
#define OPENTXS_CORE_IDENTIFIER_UNITDEFINITION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace identifier
{
class UnitDefinition;
}  // namespace identifier

using OTUnitID = Pimpl<identifier::UnitDefinition>;

OPENTXS_EXPORT auto operator==(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs
{
namespace identifier
{
class OPENTXS_EXPORT UnitDefinition : virtual public opentxs::Identifier
{
public:
    static auto Factory() -> OTUnitID;
    static auto Factory(const std::string& rhs) -> OTUnitID;
    static auto Factory(const String& rhs) -> OTUnitID;

    ~UnitDefinition() override = default;

protected:
    UnitDefinition() = default;

private:
    friend OTUnitID;

#ifndef _WIN32
    auto clone() const -> UnitDefinition* override = 0;
#endif

    UnitDefinition(const UnitDefinition&) = delete;
    UnitDefinition(UnitDefinition&&) = delete;
    auto operator=(const UnitDefinition&) -> UnitDefinition& = delete;
    auto operator=(UnitDefinition&&) -> UnitDefinition& = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
