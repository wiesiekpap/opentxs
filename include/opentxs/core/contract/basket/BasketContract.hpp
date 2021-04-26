// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_BASKET_BASKETCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_BASKET_BASKETCONTRACT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
namespace contract
{
namespace unit
{
class Basket;
}  // namespace unit
}  // namespace contract

using OTBasketContract = SharedPimpl<contract::unit::Basket>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace unit
{
class OPENTXS_EXPORT Basket : virtual public contract::Unit
{
public:
    // account number, weight
    using Subcontract = std::pair<std::string, std::uint64_t>;
    // unit definition id, subcontract
    using Subcontracts = std::map<std::string, Subcontract>;

    static OTIdentifier CalculateBasketID(
        const api::internal::Core& api,
        const proto::UnitDefinition& serialized);
    static bool FinalizeTemplate(
        const api::internal::Core& api,
        const Nym_p& nym,
        proto::UnitDefinition& serialized,
        const PasswordPrompt& reason);

    virtual OTIdentifier BasketID() const = 0;
    virtual const Subcontracts& Currencies() const = 0;
    virtual std::uint64_t Weight() const = 0;

    ~Basket() override = default;

protected:
    Basket() noexcept = default;

private:
    friend OTBasketContract;

#ifndef _WIN32
    Basket* clone() const noexcept override = 0;
#endif

    Basket(const Basket&) = delete;
    Basket(Basket&&) = delete;
    Basket& operator=(const Basket&) = delete;
    Basket& operator=(Basket&&) = delete;
};
}  // namespace unit
}  // namespace contract
}  // namespace opentxs
#endif
