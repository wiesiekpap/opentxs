// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/core/OTTrackable.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Instrument.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
OTTrackable::OTTrackable(const api::Core& core)
    : Instrument(core)
    , m_lTransactionNum(0)
    , m_SENDER_ACCT_ID(api_.Factory().Identifier())
    , m_SENDER_NYM_ID(api_.Factory().NymID())
{
    InitTrackable();
}

OTTrackable::OTTrackable(
    const api::Core& core,
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    : Instrument(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lTransactionNum(0)
    , m_SENDER_ACCT_ID(api_.Factory().Identifier())
    , m_SENDER_NYM_ID(api_.Factory().NymID())
{
    InitTrackable();
}

OTTrackable::OTTrackable(
    const api::Core& core,
    const identifier::Server& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const Identifier& ACCT_ID,
    const identifier::Nym& NYM_ID)
    : Instrument(core, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_lTransactionNum(0)
    , m_SENDER_ACCT_ID(api_.Factory().Identifier())
    , m_SENDER_NYM_ID(api_.Factory().NymID())
{
    InitTrackable();

    SetSenderAcctID(ACCT_ID);
    SetSenderNymID(NYM_ID);
}

void OTTrackable::InitTrackable()
{
    // Should never happen in practice. A child class will override it.
    m_strContractType->Set("TRACKABLE");
    m_lTransactionNum = 0;
}

auto OTTrackable::HasTransactionNum(const std::int64_t& lInput) const -> bool
{
    return lInput == m_lTransactionNum;
}

void OTTrackable::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    if (m_lTransactionNum > 0) numlistOutput.Add(m_lTransactionNum);
}

void OTTrackable::Release_Trackable()
{
    m_SENDER_ACCT_ID->Release();
    m_SENDER_NYM_ID->Release();
}

void OTTrackable::Release()
{
    Release_Trackable();
    Instrument::Release();

    // Then I call this to re-initialize everything for myself.
    InitTrackable();
}

void OTTrackable::SetSenderAcctID(const Identifier& ACCT_ID)
{
    m_SENDER_ACCT_ID = ACCT_ID;
}

void OTTrackable::SetSenderNymID(const identifier::Nym& NYM_ID)
{
    m_SENDER_NYM_ID = NYM_ID;
}

void OTTrackable::UpdateContents(const PasswordPrompt& reason) {}

OTTrackable::~OTTrackable() { Release_Trackable(); }
}  // namespace opentxs
