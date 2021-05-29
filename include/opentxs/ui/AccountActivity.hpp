// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_UI_ACCOUNTACTIVITY_HPP
#define OPENTXS_UI_ACCOUNTACTIVITY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <tuple>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/ui/List.hpp"

#ifdef SWIG
// clang-format off
%rename(UIAccountActivity) opentxs::ui::AccountActivity;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace ui
{
class AccountActivity;
class BalanceItem;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT AccountActivity : virtual public List
{
public:
    using Scale = unsigned int;

    virtual std::string AccountID() const noexcept = 0;
    virtual Amount Balance() const noexcept = 0;
    virtual int BalancePolarity() const noexcept = 0;
    virtual std::string ContractID() const noexcept = 0;
    virtual std::string DepositAddress() const noexcept = 0;
    virtual std::string DepositAddress(
        const blockchain::Type chain) const noexcept = 0;
    virtual std::vector<blockchain::Type> DepositChains() const noexcept = 0;
    virtual std::string DisplayBalance() const noexcept = 0;
    virtual std::string DisplayUnit() const noexcept = 0;
    virtual opentxs::SharedPimpl<opentxs::ui::BalanceItem> First()
        const noexcept = 0;
    virtual std::string Name() const noexcept = 0;
    virtual opentxs::SharedPimpl<opentxs::ui::BalanceItem> Next()
        const noexcept = 0;
    virtual std::string NotaryID() const noexcept = 0;
    virtual std::string NotaryName() const noexcept = 0;
    virtual bool Send(
        const Identifier& contact,
        const Amount amount,
        const std::string& memo = {}) const noexcept = 0;
    virtual bool Send(
        const Identifier& contact,
        const std::string& amount,
        const std::string& memo = {},
        Scale scale = 0) const noexcept = 0;
    virtual bool Send(
        const std::string& address,
        const Amount amount,
        const std::string& memo = {}) const noexcept = 0;
    virtual bool Send(
        const std::string& address,
        const std::string& amount,
        const std::string& memo = {},
        Scale scale = 0) const noexcept = 0;
    virtual double SyncPercentage() const noexcept = 0;
    virtual std::pair<int, int> SyncProgress() const noexcept = 0;
    virtual AccountType Type() const noexcept = 0;
    virtual contact::ContactItemType Unit() const noexcept = 0;
    virtual bool ValidateAddress(const std::string& text) const noexcept = 0;
    virtual std::string ValidateAmount(
        const std::string& text) const noexcept = 0;

    ~AccountActivity() override = default;

protected:
    AccountActivity() noexcept = default;

private:
    AccountActivity(const AccountActivity&) = delete;
    AccountActivity(AccountActivity&&) = delete;
    AccountActivity& operator=(const AccountActivity&) = delete;
    AccountActivity& operator=(AccountActivity&&) = delete;
};
}  // namespace ui
}  // namespace opentxs
#endif
