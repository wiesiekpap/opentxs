// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_WORK_HPP
#define OPENTXS_BLOCKCHAIN_WORK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Pimpl.hpp"

namespace opentxs
{
namespace blockchain
{
class Work;
}  // namespace blockchain

using OTWork = Pimpl<blockchain::Work>;

OPENTXS_EXPORT bool operator==(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept;
OPENTXS_EXPORT bool operator<(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept;
OPENTXS_EXPORT bool operator>(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept;
OPENTXS_EXPORT bool operator>=(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept;
OPENTXS_EXPORT OTWork
operator+(const OTWork& lhs, const blockchain::Work& rhs) noexcept;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class Work
{
public:
    OPENTXS_EXPORT virtual bool operator==(
        const blockchain::Work& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator!=(
        const blockchain::Work& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator<(
        const blockchain::Work& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator<=(
        const blockchain::Work& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator>(
        const blockchain::Work& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual bool operator>=(
        const blockchain::Work& rhs) const noexcept = 0;
    OPENTXS_EXPORT virtual OTWork operator+(
        const blockchain::Work& rhs) const noexcept = 0;

    OPENTXS_EXPORT virtual std::string asHex() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string Decimal() const noexcept = 0;

    OPENTXS_EXPORT virtual ~Work() = default;

protected:
    Work() noexcept = default;

private:
    friend OTWork;

    virtual Work* clone() const noexcept = 0;

    Work(const Work& rhs) = delete;
    Work(Work&& rhs) = delete;
    Work& operator=(const Work& rhs) = delete;
    Work& operator=(Work&& rhs) = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif
