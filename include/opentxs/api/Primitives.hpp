// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_PRIMITIVES_HPP
#define OPENTXS_API_PRIMITIVES_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/Secret.hpp"

namespace opentxs
{
namespace api
{
class OPENTXS_EXPORT Primitives
{
public:
    virtual auto Secret(const std::size_t bytes) const noexcept -> OTSecret = 0;
    virtual auto SecretFromBytes(const ReadView bytes) const noexcept
        -> OTSecret = 0;
    virtual auto SecretFromText(const std::string_view text) const noexcept
        -> OTSecret = 0;

    virtual ~Primitives() = default;

protected:
    Primitives() noexcept = default;

private:
    Primitives(const Primitives&) = delete;
    Primitives(Primitives&&) = delete;
    auto operator=(const Primitives&) -> Primitives& = delete;
    auto operator=(Primitives&&) -> Primitives& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
