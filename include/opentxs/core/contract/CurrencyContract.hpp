// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace contract
{
namespace unit
{
class Currency;
}  // namespace unit
}  // namespace contract

using OTCurrencyContract = SharedPimpl<contract::unit::Currency>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::contract::unit
{
class OPENTXS_EXPORT Currency : virtual public contract::Unit
{
public:
    Currency(const Currency&) = delete;
    Currency(Currency&&) = delete;
    auto operator=(const Currency&) -> Currency& = delete;
    auto operator=(Currency&&) -> Currency& = delete;

    ~Currency() override = default;

protected:
    Currency() noexcept = default;

private:
    friend OTCurrencyContract;

#ifndef _WIN32
    auto clone() const noexcept -> Currency* override = 0;
#endif
};
}  // namespace opentxs::contract::unit
