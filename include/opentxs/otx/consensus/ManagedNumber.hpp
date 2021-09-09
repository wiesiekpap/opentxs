// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_MANAGEDNUMBER_HPP
#define OPENTXS_CONSENSUS_MANAGEDNUMBER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"

namespace opentxs
{
namespace otx
{
namespace context
{
class ManagedNumber;
}  // namespace context
}  // namespace otx

using OTManagedNumber = Pimpl<otx::context::ManagedNumber>;

OPENTXS_EXPORT auto operator<(
    const OTManagedNumber& lhs,
    const OTManagedNumber& rhs) -> bool;
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
namespace context
{
class OPENTXS_EXPORT ManagedNumber
{
public:
    virtual void SetSuccess(const bool value = true) const = 0;
    virtual auto Valid() const -> bool = 0;
    virtual auto Value() const -> TransactionNumber = 0;

    virtual ~ManagedNumber() = default;

protected:
    ManagedNumber() = default;

private:
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) = delete;
    auto operator=(const ManagedNumber&) -> ManagedNumber& = delete;
    auto operator=(ManagedNumber&&) -> ManagedNumber& = delete;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
#endif
