// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <iosfwd>
#include <string_view>

#include "opentxs/Bytes.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/core/Secret.hpp"

namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::implementation
{
class Primitives final : public api::Primitives
{
public:
    auto Secret(const std::size_t bytes) const noexcept -> OTSecret final;
    auto SecretFromBytes(const ReadView bytes) const noexcept -> OTSecret final;
    auto SecretFromText(const std::string_view text) const noexcept
        -> OTSecret final;

    Primitives(const api::Crypto& crypto) noexcept;

    ~Primitives() final = default;

private:
    const api::Crypto& crypto_;

    Primitives() = delete;
    Primitives(const Primitives&) = delete;
    Primitives(Primitives&&) = delete;
    auto operator=(const Primitives&) -> Primitives& = delete;
    auto operator=(Primitives&&) -> Primitives& = delete;
};
}  // namespace opentxs::api::implementation
