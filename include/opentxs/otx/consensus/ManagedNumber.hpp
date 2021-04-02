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

OPENTXS_EXPORT bool operator<(
    const OTManagedNumber& lhs,
    const OTManagedNumber& rhs);
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
namespace context
{
class ManagedNumber
{
public:
    OPENTXS_EXPORT virtual void SetSuccess(const bool value = true) const = 0;
    OPENTXS_EXPORT virtual bool Valid() const = 0;
    OPENTXS_EXPORT virtual TransactionNumber Value() const = 0;

    OPENTXS_EXPORT virtual ~ManagedNumber() = default;

protected:
    ManagedNumber() = default;

private:
    ManagedNumber(const ManagedNumber&) = delete;
    ManagedNumber(ManagedNumber&& rhs) = delete;
    ManagedNumber& operator=(const ManagedNumber&) = delete;
    ManagedNumber& operator=(ManagedNumber&&) = delete;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
#endif
