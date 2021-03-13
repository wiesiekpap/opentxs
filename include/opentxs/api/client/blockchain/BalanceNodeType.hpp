// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODETYPE_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODETYPE_HPP

#include "opentxs/api/client/blockchain/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
enum class BalanceNodeType : std::uint16_t {
    Error = 0,
    HD = 1,
    PaymentCode = 2,
    Imported = 3,
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
