// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_ZAP_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_ZAP_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
enum class Mechanism : int {
    Unknown = 0,
    Null = 1,
    Plain = 2,
    Curve = 3,
};

enum class Status : int {
    Unknown = 0,
    Success = 200,
    TemporaryError = 300,
    AuthFailure = 400,
    SystemError = 500,
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
