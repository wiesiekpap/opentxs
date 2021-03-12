// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIST_LANGUAGE_HPP
#define OPENTXS_CRYPTO_LIST_LANGUAGE_HPP

#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs
{
namespace crypto
{
enum class Language : std::uint8_t {
    none = 0,
    en = 1,
};
}  // namespace crypto
}  // namespace opentxs
#endif
