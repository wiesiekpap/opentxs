// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_PRIMITIVES_HPP
#define OPENTXS_API_PRIMITIVES_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/core/Secret.hpp"

namespace opentxs
{
namespace api
{
class Primitives
{
public:
    OPENTXS_EXPORT virtual OTSecret Secret(
        const std::size_t bytes) const noexcept = 0;
    OPENTXS_EXPORT virtual OTSecret SecretFromBytes(
        const ReadView bytes) const noexcept = 0;
    OPENTXS_EXPORT virtual OTSecret SecretFromText(
        const std::string_view text) const noexcept = 0;

    virtual ~Primitives() = default;

protected:
    Primitives() noexcept = default;

private:
    Primitives(const Primitives&) = delete;
    Primitives(Primitives&&) = delete;
    Primitives& operator=(const Primitives&) = delete;
    Primitives& operator=(Primitives&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
