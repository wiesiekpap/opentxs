// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_RPC_CONTACT_EVENT_TYPE_HPP
#define OPENTXS_RPC_CONTACT_EVENT_TYPE_HPP

#include "opentxs/rpc/Types.hpp"  // IWYU pragma: associated

#include <limits>

namespace opentxs
{
namespace rpc
{
enum class ContactEventType : TypeEnum {
    error = 0,
    incoming_message = 1,
    outgoing_message = 2,
    incoming_payment = 3,
    outgoing_payment = 4,
};
}  // namespace rpc
}  // namespace opentxs
#endif
