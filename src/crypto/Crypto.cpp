// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

#include <map>

#include "opentxs/crypto/SeedStyle.hpp"

namespace opentxs
{
auto print(crypto::SeedStyle type) noexcept -> std::string
{
    using Type = crypto::SeedStyle;
    static const auto map = std::map<Type, std::string>{
        {Type::Error, "invalid"},
        {Type::BIP32, "BIP-32"},
        {Type::BIP39, "BIP-39"},
        {Type::PKT, "pktwallet"},
    };

    try {

        return map.at(type);
    } catch (...) {

        return map.at(Type::Error);
    }
}
}  // namespace opentxs
