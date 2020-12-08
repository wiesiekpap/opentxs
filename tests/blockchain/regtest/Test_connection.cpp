// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <iosfwd>

namespace
{
TEST_F(Regtest_fixture, init_opentxs) {}

TEST_F(Regtest_fixture, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture, client_disconnection_timeout)
{
    EXPECT_TRUE(client_.Blockchain().Stop(chain_));

    const auto start = ot::Clock::now();
    const auto limit = std::chrono::minutes{1};
    const auto& chain = miner_.Blockchain().GetChain(chain_);
    auto count = std::size_t{1};

    while ((ot::Clock::now() - start) < limit) {
        count = chain.GetPeerCount();

        if (0u == count) { break; }

        ot::Sleep(std::chrono::seconds(1));
    }

    EXPECT_EQ(count, 0);
}

TEST_F(Regtest_fixture, shutdown) { Shutdown(); }
}  // namespace
