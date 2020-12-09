// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>

#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep

namespace
{
TEST_F(Regtest_fixture_normal, init_opentxs) {}

TEST_F(Regtest_fixture_normal, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_normal, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_normal, mine) { EXPECT_TRUE(Mine(0, 10)); }

TEST_F(Regtest_fixture_normal, extend) { EXPECT_TRUE(Mine(10, 5)); }

TEST_F(Regtest_fixture_normal, reorg) { EXPECT_TRUE(Mine(12, 5)); }

TEST_F(Regtest_fixture_normal, shutdown) { Shutdown(); }
}  // namespace
