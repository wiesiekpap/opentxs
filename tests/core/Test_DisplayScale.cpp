// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "display/Definition.hpp"
#include "display/Scale.hpp"
#include "opentxs/Types.hpp"

TEST(DisplayScale, usd)
{
    const auto usd = opentxs::display::Definition{{
        {u8"dollars", {u8"$", u8"", {{10, 3}}, 2, 3}},
        {u8"cents", {u8"", u8"¢", {{10, 1}}, 0, 1}},
        {u8"millions", {u8"$", u8"MM", {{10, 9}}, 0, 9}},
        {u8"mills", {u8"", u8"₥", {{10, 0}}, 0, 0}},
    }};

    const auto scales = usd.GetScales();
    auto it = scales.begin();

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 0);
        EXPECT_EQ(name, "dollars");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 1);
        EXPECT_EQ(name, "cents");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 2);
        EXPECT_EQ(name, "millions");
    }

    const auto amount1 = opentxs::Amount{14000000000};
    const auto amount2 = opentxs::Amount{14000000880};
    const auto amount3 = opentxs::Amount{14005000000};
    const auto amount4 = opentxs::Amount{14015000000};

    EXPECT_EQ(usd.Format(amount1), usd.Format(amount1, 0));
    EXPECT_EQ(usd.Format(amount1, 0), std::string{u8"$14,000,000.00"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(usd.Format(amount1, 1), std::string{u8"1,400,000,000 ¢"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(usd.Format(amount1, 2), std::string{u8"$14 MM"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(usd.Format(amount1, 3), std::string{u8"14,000,000,000 ₥"});
    EXPECT_EQ(usd.Import(usd.Format(amount1, 3), 3), amount1);

    EXPECT_EQ(usd.Format(amount2, 0), std::string{u8"$14,000,000.88"});
    EXPECT_EQ(usd.Format(amount2, 1), std::string{u8"1,400,000,088 ¢"});
    EXPECT_EQ(
        usd.Format(amount2, 2), std::string{u8"$14.000\u202F000\u202F88 MM"});
    EXPECT_EQ(usd.Format(amount2, 2, std::nullopt, 2), std::string{u8"$14 MM"});
    EXPECT_EQ(usd.Format(amount2, 2, 0, 2), std::string{u8"$14 MM"});
    EXPECT_EQ(usd.Format(amount2, 2, 1, 2), std::string{u8"$14.0 MM"});

    EXPECT_EQ(usd.Format(amount3, 2, 1, 2), std::string{u8"$14.0 MM"});

    EXPECT_EQ(usd.Format(amount4, 2, 1, 2), std::string{u8"$14.02 MM"});

    const auto commaTest = std::vector<std::pair<opentxs::Amount, std::string>>{
        {00, u8"$0"},
        {10, u8"$0.01"},
        {1000, u8"$1"},
        {1010, u8"$1.01"},
        {10000, u8"$10"},
        {10010, u8"$10.01"},
        {100000, u8"$100"},
        {100010, u8"$100.01"},
        {1000000, u8"$1,000"},
        {1000010, u8"$1,000.01"},
        {10000000, u8"$10,000"},
        {10000010, u8"$10,000.01"},
        {100000000, u8"$100,000"},
        {100000010, u8"$100,000.01"},
        {1000000000, u8"$1,000,000"},
        {1000000010, u8"$1,000,000.01"},
        {10000000000, u8"$10,000,000"},
        {10000000010, u8"$10,000,000.01"},
        {100000000000, u8"$100,000,000"},
        {100000000010, u8"$100,000,000.01"},
        {1000000000000, u8"$1,000,000,000"},
        {1000000000010, u8"$1,000,000,000.01"},
        {10000000000000, u8"$10,000,000,000"},
        {10000000000010, u8"$10,000,000,000.01"},
        {100000000000000, u8"$100,000,000,000"},
        {100000000000010, u8"$100,000,000,000.01"},
    };

    for (const auto& [amount, expected] : commaTest) {
        EXPECT_EQ(usd.Format(amount, 0, 0), expected);
    }
}

TEST(DisplayScale, btc)
{
    const auto btc = opentxs::display::Definition{{
        {u8"BTC", {"", u8"₿", {{10, 8}}, 0, 8}},
        {u8"mBTC", {"", u8"mBTC", {{10, 5}}, 0, 5}},
        {u8"μBTC", {"", u8"μBTC", {{10, 2}}, 0, 2}},
    }};

    const auto scales = btc.GetScales();
    auto it = scales.begin();

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 0);
        EXPECT_EQ(name, u8"BTC");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 1);
        EXPECT_EQ(name, u8"mBTC");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 2);
        EXPECT_EQ(name, u8"μBTC");
    }

    const auto amount1 = opentxs::Amount{100000000};
    const auto amount2 = opentxs::Amount{1};
    const auto amount3 = opentxs::Amount{2099999999999999};

    EXPECT_EQ(btc.Format(amount1, 0), std::string{u8"1 ₿"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(btc.Format(amount1, 1), std::string{u8"1,000 mBTC"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(btc.Format(amount1, 2), std::string{u8"1,000,000 μBTC"});
    EXPECT_EQ(btc.Import(btc.Format(amount1, 2), 2), amount1);

    EXPECT_EQ(
        btc.Format(amount2, 0), std::string{u8"0.000\u202F000\u202F01 ₿"});
    EXPECT_EQ(btc.Format(amount2, 1), std::string{u8"0.000\u202F01 mBTC"});
    EXPECT_EQ(btc.Format(amount2, 2), std::string{u8"0.01 μBTC"});

    EXPECT_EQ(
        btc.Format(amount3, 0),
        std::string{u8"20,999,999.999\u202F999\u202F99 ₿"});
    EXPECT_EQ(btc.Import(btc.Format(amount3, 0), 0), amount3);
}

TEST(DisplayScale, pkt)
{
    const auto pkt = opentxs::display::Definition{{
        {u8"PKT", {"", u8"PKT", {{2, 30}}, 0, 11}},
        {u8"mPKT", {"", u8"mPKT", {{2, 30}, {10, -3}}, 0, 8}},
        {u8"μPKT", {"", u8"μPKT", {{2, 30}, {10, -6}}, 0, 5}},
        {u8"nPKT", {"", u8"nPKT", {{2, 30}, {10, -9}}, 0, 2}},
        {u8"pack", {"", u8"pack", {{10, 0}}, 0, 2}},
    }};

    const auto scales = pkt.GetScales();
    auto it = scales.begin();

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 0);
        EXPECT_EQ(name, u8"PKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 1);
        EXPECT_EQ(name, u8"mPKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 2);
        EXPECT_EQ(name, u8"μPKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 3);
        EXPECT_EQ(name, u8"nPKT");
    }

    std::advance(it, 1);

    {
        const auto& [index, name] = *it;

        EXPECT_EQ(index, 4);
        EXPECT_EQ(name, u8"pack");
    }

    const auto amount1 = opentxs::Amount{1073741824};
    const auto amount2 = opentxs::Amount{1};

    EXPECT_EQ(pkt.Format(amount1, 0), std::string{u8"1 PKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 0), 0), amount1);
    EXPECT_EQ(pkt.Format(amount1, 1), std::string{u8"1,000 mPKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 1), 1), amount1);
    EXPECT_EQ(pkt.Format(amount1, 2), std::string{u8"1,000,000 μPKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 2), 2), amount1);
    EXPECT_EQ(pkt.Format(amount1, 3), std::string{u8"1,000,000,000 nPKT"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 3), 3), amount1);
    EXPECT_EQ(pkt.Format(amount1, 4), std::string{u8"1,073,741,824 pack"});
    EXPECT_EQ(pkt.Import(pkt.Format(amount1, 4), 4), amount1);

    EXPECT_EQ(
        pkt.Format(amount2, 0),
        std::string{u8"0.000\u202F000\u202F000\u202F93 PKT"});
    EXPECT_EQ(
        pkt.Format(amount2, 1), std::string{u8"0.000\u202F000\u202F93 mPKT"});
    EXPECT_EQ(pkt.Format(amount2, 2), std::string{u8"0.000\u202F93 μPKT"});
    EXPECT_EQ(pkt.Format(amount2, 3), std::string{u8"0.93 nPKT"});
    EXPECT_EQ(pkt.Format(amount2, 4), std::string{u8"1 pack"});
}
