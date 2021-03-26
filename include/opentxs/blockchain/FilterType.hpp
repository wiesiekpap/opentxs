// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_FILTERTYPE_HPP
#define OPENTXS_BLOCKCHAIN_FILTERTYPE_HPP

#include "opentxs/blockchain/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs
{
namespace blockchain
{
namespace filter
{
enum class Type : TypeEnum {
    Basic_BIP158 = 0,
    Basic_BCHVariant = 1,
    ES = 88,
    Unknown = std::numeric_limits<TypeEnum>::max(),
};
}  // namespace filter
}  // namespace blockchain
}  // namespace opentxs
#endif
