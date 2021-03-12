// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_MINT_HPP
#define OPENTXS_BLIND_MINT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <ctime>

#if OT_CASH
#include "opentxs/core/Contract.hpp"

namespace opentxs
{
namespace blind
{
class Mint : virtual public Contract
{
public:
    OPENTXS_EXPORT virtual OTIdentifier AccountID() const = 0;
    OPENTXS_EXPORT virtual bool Expired() const = 0;
    OPENTXS_EXPORT virtual std::int64_t GetDenomination(
        std::int32_t nIndex) const = 0;
    OPENTXS_EXPORT virtual std::int32_t GetDenominationCount() const = 0;
    OPENTXS_EXPORT virtual Time GetExpiration() const = 0;
    OPENTXS_EXPORT virtual std::int64_t GetLargestDenomination(
        std::int64_t lAmount) const = 0;
    OPENTXS_EXPORT virtual bool GetPrivate(
        Armored& theArmor,
        std::int64_t lDenomination) const = 0;
    OPENTXS_EXPORT virtual bool GetPublic(
        Armored& theArmor,
        std::int64_t lDenomination) const = 0;
    OPENTXS_EXPORT virtual std::int32_t GetSeries() const = 0;
    OPENTXS_EXPORT virtual Time GetValidFrom() const = 0;
    OPENTXS_EXPORT virtual Time GetValidTo() const = 0;
    OPENTXS_EXPORT virtual const identifier::UnitDefinition&
    InstrumentDefinitionID() const = 0;

    OPENTXS_EXPORT virtual bool AddDenomination(
        const identity::Nym& theNotary,
        const std::int64_t denomination,
        const std::size_t keySize,
        const PasswordPrompt& reason) = 0;
    OPENTXS_EXPORT virtual void GenerateNewMint(
        const api::Wallet& wallet,
        const std::int32_t nSeries,
        const Time VALID_FROM,
        const Time VALID_TO,
        const Time MINT_EXPIRATION,
        const identifier::UnitDefinition& theInstrumentDefinitionID,
        const identifier::Server& theNotaryID,
        const identity::Nym& theNotary,
        const std::int64_t nDenom1,
        const std::int64_t nDenom2,
        const std::int64_t nDenom3,
        const std::int64_t nDenom4,
        const std::int64_t nDenom5,
        const std::int64_t nDenom6,
        const std::int64_t nDenom7,
        const std::int64_t nDenom8,
        const std::int64_t nDenom9,
        const std::int64_t nDenom10,
        const std::size_t keySize,
        const PasswordPrompt& reason) = 0;
    OPENTXS_EXPORT virtual bool LoadMint(const char* szAppend = nullptr) = 0;
    OPENTXS_EXPORT virtual void Release_Mint() = 0;
    OPENTXS_EXPORT virtual void ReleaseDenominations() = 0;
    OPENTXS_EXPORT virtual bool SaveMint(const char* szAppend = nullptr) = 0;
    OPENTXS_EXPORT virtual void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) = 0;
    OPENTXS_EXPORT virtual void SetSavePrivateKeys(bool bDoIt = true) = 0;
    OPENTXS_EXPORT virtual bool SignToken(
        const identity::Nym& notary,
        blind::Token& token,
        const PasswordPrompt& reason) = 0;
    OPENTXS_EXPORT virtual bool VerifyMint(
        const identity::Nym& theOperator) = 0;
    OPENTXS_EXPORT virtual bool VerifyToken(
        const identity::Nym& notary,
        const blind::Token& token,
        const PasswordPrompt& reason) = 0;

    OPENTXS_EXPORT ~Mint() override = default;

protected:
    Mint() = default;

private:
    Mint(const Mint&) = delete;
    Mint(Mint&&) = delete;
    Mint& operator=(const Mint&) = delete;
    Mint& operator=(Mint&&) = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif  // OT_CASH
#endif
