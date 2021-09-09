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

OPENTXS_EXPORT auto operator==(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator+(
    const OTWork& lhs,
    const blockchain::Work& rhs) noexcept -> OTWork;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
class OPENTXS_EXPORT Work
{
public:
    virtual auto operator==(const blockchain::Work& rhs) const noexcept
        -> bool = 0;
    virtual auto operator!=(const blockchain::Work& rhs) const noexcept
        -> bool = 0;
    virtual auto operator<(const blockchain::Work& rhs) const noexcept
        -> bool = 0;
    virtual auto operator<=(const blockchain::Work& rhs) const noexcept
        -> bool = 0;
    virtual auto operator>(const blockchain::Work& rhs) const noexcept
        -> bool = 0;
    virtual auto operator>=(const blockchain::Work& rhs) const noexcept
        -> bool = 0;
    virtual auto operator+(const blockchain::Work& rhs) const noexcept
        -> OTWork = 0;

    virtual auto asHex() const noexcept -> std::string = 0;
    virtual auto Decimal() const noexcept -> std::string = 0;

    virtual ~Work() = default;

protected:
    Work() noexcept = default;

private:
    friend OTWork;

    virtual auto clone() const noexcept -> Work* = 0;

    Work(const Work& rhs) = delete;
    Work(Work&& rhs) = delete;
    auto operator=(const Work& rhs) -> Work& = delete;
    auto operator=(Work&& rhs) -> Work& = delete;
};
}  // namespace blockchain
}  // namespace opentxs
#endif
