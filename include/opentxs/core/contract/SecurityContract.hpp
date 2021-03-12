// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_SECURITYCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_SECURITYCONTRACT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"

namespace opentxs
{
namespace contract
{
namespace unit
{
class Security;
}  // namespace unit
}  // namespace contract

using OTSecurityContract = SharedPimpl<contract::unit::Security>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace unit
{
class Security : virtual public contract::Unit
{
public:
    OPENTXS_EXPORT ~Security() override = default;

protected:
    Security() noexcept = default;

private:
    friend OTSecurityContract;

#ifndef _WIN32
    OPENTXS_EXPORT Security* clone() const noexcept override = 0;
#endif

    Security(const Security&) = delete;
    Security(Security&&) = delete;
    Security& operator=(const Security&) = delete;
    Security& operator=(Security&&) = delete;
};
}  // namespace unit
}  // namespace contract
}  // namespace opentxs
#endif
