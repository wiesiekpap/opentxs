// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_UNITDEFINITION_HPP
#define OPENTXS_CORE_CONTRACT_UNITDEFINITION_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Types.hpp"

namespace opentxs
{
namespace contract
{
class Unit;
}  // namespace contract

namespace proto
{
class UnitDefinition;
}  // namespace proto

class Account;
class AccountVisitor;
class PasswordPrompt;

using OTUnitDefinition = SharedPimpl<contract::Unit>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
class OPENTXS_EXPORT Unit : virtual public opentxs::contract::Signable
{
public:
    using SerializedType = proto::UnitDefinition;

    static const VersionNumber DefaultVersion;
    static const VersionNumber MaxVersion;

    static auto formatLongAmount(
        const Amount lValue,
        const std::int32_t nFactor = 100,
        const std::int32_t nPower = 2,
        const char* szCurrencySymbol = "",
        const char* szThousandSeparator = ",",
        const char* szDecimalPoint = ".") -> std::string;
    static auto ParseFormatted(
        Amount& lResult,
        const std::string& str_input,
        const std::int32_t nFactor = 100,
        const std::int32_t nPower = 2,
        const char* szThousandSeparator = ",",
        const char* szDecimalPoint = ".") -> bool;
    static auto ValidUnits(
        const VersionNumber version = DefaultVersion) noexcept
        -> std::set<contact::ContactItemType>;

    virtual auto AddAccountRecord(
        const std::string& dataFolder,
        const Account& theAccount) const -> bool = 0;
    virtual auto DecimalPower() const -> std::int32_t = 0;
    virtual auto DisplayStatistics(String& strContents) const -> bool = 0;
    virtual auto EraseAccountRecord(
        const std::string& dataFolder,
        const Identifier& theAcctID) const -> bool = 0;
    virtual auto FormatAmountLocale(
        Amount amount,
        std::string& str_output,
        const std::string& str_thousand,
        const std::string& str_decimal) const -> bool = 0;
    virtual auto FormatAmountLocale(Amount amount, std::string& str_output)
        const -> bool = 0;
    virtual auto FormatAmountWithoutSymbolLocale(
        Amount amount,
        std::string& str_output,
        const std::string& str_thousand,
        const std::string& str_decimal) const -> bool = 0;
    virtual auto FormatAmountWithoutSymbolLocale(
        Amount amount,
        std::string& str_output) const -> bool = 0;
    virtual auto FractionalUnitName() const -> std::string = 0;
    virtual auto GetCurrencyName() const -> const std::string& = 0;
    virtual auto GetCurrencySymbol() const -> const std::string& = 0;
    using Signable::Serialize;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        SerializedType&,
        bool includeNym = false) const -> bool = 0;
    virtual auto Serialize(AllocateOutput destination, bool includeNym = false)
        const -> bool = 0;
    virtual auto StringToAmountLocale(
        Amount& amount,
        const std::string& str_input,
        const std::string& str_thousand,
        const std::string& str_decimal) const -> bool = 0;
    virtual auto TLA() const -> std::string = 0;
    virtual auto Type() const -> contract::UnitType = 0;
    virtual auto UnitOfAccount() const -> contact::ContactItemType = 0;
    virtual auto VisitAccountRecords(
        const std::string& dataFolder,
        AccountVisitor& visitor,
        const PasswordPrompt& reason) const -> bool = 0;

    virtual void InitAlias(const std::string& alias) = 0;

    ~Unit() override = default;

protected:
    Unit() noexcept = default;

private:
    friend OTUnitDefinition;

#ifndef _WIN32
    auto clone() const noexcept -> Unit* override = 0;
#endif

    Unit(const Unit&) = delete;
    Unit(Unit&&) = delete;
    auto operator=(const Unit&) -> Unit& = delete;
    auto operator=(Unit&&) -> Unit& = delete;
};
}  // namespace contract
}  // namespace opentxs
#endif
