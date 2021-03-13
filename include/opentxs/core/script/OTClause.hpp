// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTCLAUSE_HPP
#define OPENTXS_CORE_SCRIPT_OTCLAUSE_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/core/String.hpp"

namespace opentxs
{
class OTBylaw;
class Tag;

class OTClause
{
    OTString m_strName;  // Name of this Clause.
    OTString m_strCode;  // script code.
    OTBylaw* m_pBylaw;   // the Bylaw that this clause belongs to.

    OTClause(const OTClause&) = delete;
    OTClause(OTClause&&) = delete;
    OTClause& operator=(const OTClause&) = delete;
    OTClause& operator=(OTClause&&) = delete;

public:
    void SetBylaw(OTBylaw& theBylaw) { m_pBylaw = &theBylaw; }

    OPENTXS_EXPORT const String& GetName() const { return m_strName; }

    OTBylaw* GetBylaw() const { return m_pBylaw; }

    OPENTXS_EXPORT const char* GetCode() const;

    OPENTXS_EXPORT void SetCode(const std::string& str_code);

    bool Compare(const OTClause& rhs) const;

    OTClause();
    OTClause(const char* szName, const char* szCode);
    virtual ~OTClause();

    void Serialize(Tag& parent) const;
};
}  // namespace opentxs
#endif
