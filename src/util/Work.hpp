// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
constexpr auto OT_ZMQ_INTERNAL_SIGNAL = OTZMQWorkType{32768};
constexpr auto OT_ZMQ_HIGHEST_SIGNAL = OTZMQWorkType{65535};

// clang-format off
constexpr auto OT_ZMQ_STATE_MACHINE_SIGNAL =             OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 0};
constexpr auto OT_ZMQ_SEND_SIGNAL =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 1};
constexpr auto OT_ZMQ_RECEIVE_SIGNAL =                   OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 2};
constexpr auto OT_ZMQ_NEW_FILTER_SIGNAL =                OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 3};
constexpr auto OT_ZMQ_NEW_BLOCKCHAIN_WALLET_KEY_SIGNAL = OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 4};
constexpr auto OT_ZMQ_INIT_SIGNAL =                      OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 5};
constexpr auto OT_ZMQ_NEW_FULL_BLOCK_SIGNAL =            OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 6};
constexpr auto OT_ZMQ_SYNC_DATA_SIGNAL =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 7};
constexpr auto OT_ZMQ_HEARTBEAT_SIGNAL =                 OTZMQWorkType{OT_ZMQ_HIGHEST_SIGNAL - 8};
// clang-format on
}  // namespace opentxs
