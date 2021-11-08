// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/Types.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <limits>

#include "opentxs/contact/Types.hpp"

namespace opentxs
{
namespace core
{
enum class UnitType : std::uint8_t {
    Error = 0,
    BTC = 1,
    BCH = 2,
    ETH = 3,
    XRP = 4,
    LTC = 5,
    DAO = 6,
    XEM = 7,
    DASH = 8,
    MAID = 9,
    LSK = 10,
    DOGE = 11,
    DGD = 12,
    XMR = 13,
    WAVES = 14,
    NXT = 15,
    SC = 16,
    STEEM = 17,
    AMP = 18,
    XLM = 19,
    FCT = 20,
    BTS = 21,
    USD = 22,
    EUR = 23,
    GBP = 24,
    INR = 25,
    AUD = 26,
    CAD = 27,
    SGD = 28,
    CHF = 29,
    MYR = 30,
    JPY = 31,
    CNY = 32,
    NZD = 33,
    THB = 34,
    HUF = 35,
    AED = 36,
    HKD = 37,
    MXN = 38,
    ZAR = 39,
    PHP = 40,
    SEC = 41,
    PKT = 42,
    TNBTC = 43,
    TNBCH = 44,
    TNXRP = 45,
    TNLTX = 46,
    TNXEM = 47,
    TNDASH = 48,
    TNMAID = 49,
    TNLSK = 50,
    TNDOGE = 51,
    TNXMR = 52,
    TNWAVES = 53,
    TNNXT = 54,
    TNSC = 55,
    TNSTEEM = 56,
    TNPKT = 57,
    Ethereum_Olympic = 58,
    Ethereum_Classic = 59,
    Ethereum_Expanse = 60,
    Ethereum_Morden = 61,
    Ethereum_Ropsten = 62,
    Ethereum_Rinkeby = 63,
    Ethereum_Kovan = 64,
    Ethereum_Sokol = 65,
    Ethereum_POA = 66,
    Regtest = 67,

    Unknown = std::numeric_limits<std::uint8_t>::max(),
};

}  // namespace core
}  // namespace opentxs
