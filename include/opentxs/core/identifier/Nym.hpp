// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_NYM_HPP
#define OPENTXS_CORE_IDENTIFIER_NYM_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::identifier::Nym::Factory;
%extend opentxs::identifier::Nym {
    static OTNymID Factory()
    {
        return opentxs::identifier::Nym::Factory();
    }
    static OTNymID Factory(
        const std::string& rhs)
    {
        return opentxs::identifier::Nym::Factory(rhs);
    }
}
%rename (NymID) opentxs::identifier::Nym;
%template(OTNymID) opentxs::Pimpl<opentxs::identifier::Nym>;
// clang-format on
#endif

namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

using OTNymID = Pimpl<identifier::Nym>;

#ifndef SWIG
OPENTXS_EXPORT bool operator==(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator<(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator>(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator>=(
    const OTNymID& lhs,
    const opentxs::Identifier& rhs) noexcept;
#endif
}  // namespace opentxs

namespace opentxs
{
namespace identifier
{
class OPENTXS_EXPORT Nym : virtual public opentxs::Identifier
{
public:
#ifndef SWIG
    static OTNymID Factory();
    static OTNymID Factory(const std::string& rhs);
    static OTNymID Factory(const String& rhs);
    static OTNymID Factory(const identity::Nym& nym);
#endif

    ~Nym() override = default;

protected:
    Nym() = default;

private:
    friend OTNymID;

#ifndef _WIN32
    Nym* clone() const override = 0;
#endif
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
