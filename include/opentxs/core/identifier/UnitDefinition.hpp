// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_UNITDEFINITION_HPP
#define OPENTXS_CORE_IDENTIFIER_UNITDEFINITION_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::identifier::UnitDefinition::Factory;
%extend opentxs::identifier::UnitDefinition {
    static OTUnitID Factory()
    {
        return opentxs::identifier::UnitDefinition::Factory();
    }
    static OTUnitID Factory(
        const std::string& rhs)
    {
        return opentxs::identifier::UnitDefinition::Factory(rhs);
    }
}
%rename (UnitDefinitionID) opentxs::identifier::UnitDefinition;
%template(OTUnitID) opentxs::Pimpl<opentxs::identifier::UnitDefinition>;
// clang-format on
#endif

namespace opentxs
{
namespace identifier
{
class UnitDefinition;
}  // namespace identifier

using OTUnitID = Pimpl<identifier::UnitDefinition>;

#ifndef SWIG
OPENTXS_EXPORT bool operator==(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator<(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator>(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator>=(
    const OTUnitID& lhs,
    const opentxs::Identifier& rhs) noexcept;
#endif
}  // namespace opentxs

namespace opentxs
{
namespace identifier
{
class UnitDefinition : virtual public opentxs::Identifier
{
public:
#ifndef SWIG
    OPENTXS_EXPORT static OTUnitID Factory();
    OPENTXS_EXPORT static OTUnitID Factory(const std::string& rhs);
    OPENTXS_EXPORT static OTUnitID Factory(const String& rhs);
#endif

    OPENTXS_EXPORT ~UnitDefinition() override = default;

protected:
    UnitDefinition() = default;

private:
    friend OTUnitID;

#ifndef _WIN32
    UnitDefinition* clone() const override = 0;
#endif

    UnitDefinition(const UnitDefinition&) = delete;
    UnitDefinition(UnitDefinition&&) = delete;
    UnitDefinition& operator=(const UnitDefinition&) = delete;
    UnitDefinition& operator=(UnitDefinition&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
