// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_UTIL_HPP
#define OPENTXS_API_CRYPTO_UTIL_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs
{
namespace api
{
namespace crypto
{
class Util
{
public:
    virtual auto RandomizeMemory(void* destination, const std::size_t size)
        const -> bool = 0;

    virtual ~Util() = default;

protected:
    Util() = default;

private:
    Util(const Util&) = delete;
    Util(Util&&) = delete;
    auto operator=(const Util&) -> Util& = delete;
    auto operator=(Util&&) -> Util& = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif
