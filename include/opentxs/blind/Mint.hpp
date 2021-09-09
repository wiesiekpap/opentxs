// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLIND_MINT_HPP
#define OPENTXS_BLIND_MINT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <ctime>

#if OT_CASH
#include "opentxs/core/Contract.hpp"

namespace opentxs
{
namespace api
{
class Wallet;
}  // namespace api

namespace blind
{
class Token;
}  // namespace blind

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier
}  // namespace opentxs

namespace opentxs
{
namespace blind
{
class OPENTXS_EXPORT Mint : virtual public Contract
{
public:
    virtual auto AccountID() const -> OTIdentifier = 0;
    virtual auto Expired() const -> bool = 0;
    virtual auto GetDenomination(std::int32_t nIndex) const -> std::int64_t = 0;
    virtual auto GetDenominationCount() const -> std::int32_t = 0;
    virtual auto GetExpiration() const -> Time = 0;
    virtual auto GetLargestDenomination(std::int64_t lAmount) const
        -> std::int64_t = 0;
    virtual auto GetPrivate(Armored& theArmor, std::int64_t lDenomination) const
        -> bool = 0;
    virtual auto GetPublic(Armored& theArmor, std::int64_t lDenomination) const
        -> bool = 0;
    virtual auto GetSeries() const -> std::int32_t = 0;
    virtual auto GetValidFrom() const -> Time = 0;
    virtual auto GetValidTo() const -> Time = 0;
    virtual auto InstrumentDefinitionID() const
        -> const identifier::UnitDefinition& = 0;

    virtual auto AddDenomination(
        const identity::Nym& theNotary,
        const std::int64_t denomination,
        const std::size_t keySize,
        const PasswordPrompt& reason) -> bool = 0;
    virtual void GenerateNewMint(
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
    virtual auto LoadMint(const char* szAppend = nullptr) -> bool = 0;
    virtual void Release_Mint() = 0;
    virtual void ReleaseDenominations() = 0;
    virtual auto SaveMint(const char* szAppend = nullptr) -> bool = 0;
    virtual void SetInstrumentDefinitionID(
        const identifier::UnitDefinition& newID) = 0;
    virtual void SetSavePrivateKeys(bool bDoIt = true) = 0;
    virtual auto SignToken(
        const identity::Nym& notary,
        blind::Token& token,
        const PasswordPrompt& reason) -> bool = 0;
    virtual auto VerifyMint(const identity::Nym& theOperator) -> bool = 0;
    virtual auto VerifyToken(
        const identity::Nym& notary,
        const blind::Token& token,
        const PasswordPrompt& reason) -> bool = 0;

    ~Mint() override = default;

protected:
    Mint() = default;

private:
    Mint(const Mint&) = delete;
    Mint(Mint&&) = delete;
    auto operator=(const Mint&) -> Mint& = delete;
    auto operator=(Mint&&) -> Mint& = delete;
};
}  // namespace blind
}  // namespace opentxs
#endif  // OT_CASH
#endif
