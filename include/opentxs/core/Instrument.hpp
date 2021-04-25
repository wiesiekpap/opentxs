// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_INSTRUMENT_HPP
#define OPENTXS_CORE_INSTRUMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <irrxml/irrXML.hpp>
#include <cstdint>

#include "opentxs/Types.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/core/script/OTScriptable.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

class OPENTXS_EXPORT Instrument : public OTScriptable
{
public:
    void Release() override;

    void Release_Instrument();
    bool VerifyCurrentDate();  // Verify whether the CURRENT date
                               // is WITHIN the VALID FROM / TO
                               // dates.
    bool IsExpired();          // Verify whether the CURRENT date is
                               // AFTER the the "VALID TO" date.
    inline Time GetValidFrom() const { return m_VALID_FROM; }
    inline Time GetValidTo() const { return m_VALID_TO; }

    inline const identifier::UnitDefinition& GetInstrumentDefinitionID() const
    {
        return m_InstrumentDefinitionID;
    }
    inline const identifier::Server& GetNotaryID() const { return m_NotaryID; }
    void InitInstrument();

    ~Instrument() override;

protected:
    OTUnitID m_InstrumentDefinitionID;
    OTServerID m_NotaryID;
    // Expiration Date (valid from/to date)
    // The date, in seconds, when the instrument is valid FROM.
    Time m_VALID_FROM;
    // The date, in seconds, when the instrument expires.
    Time m_VALID_TO;

    std::int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;

    inline void SetValidFrom(const Time TIME_FROM) { m_VALID_FROM = TIME_FROM; }
    inline void SetValidTo(const Time TIME_TO) { m_VALID_TO = TIME_TO; }
    inline void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    {
        m_InstrumentDefinitionID = INSTRUMENT_DEFINITION_ID;
    }
    inline void SetNotaryID(const identifier::Server& NOTARY_ID)
    {
        m_NotaryID = NOTARY_ID;
    }

    Instrument(const api::internal::Core& core);
    Instrument(
        const api::internal::Core& core,
        const identifier::Server& NOTARY_ID,
        const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID);

private:
    Instrument() = delete;
};
}  // namespace opentxs
#endif
