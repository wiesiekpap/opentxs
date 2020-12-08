// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/client/HeaderOracle.hpp"

namespace
{
TEST_F(Regtest_fixture, init_opentxs) {}

TEST_F(Regtest_fixture, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture, mine_block_sequence)
{
    constexpr auto blocksToMine{15};

    auto blockFuture = block_.GetFuture(blocksToMine);
    auto walletFuture = wallet_.GetFuture(blocksToMine);
    const auto& network = miner_.Blockchain().GetChain(chain_);
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader = [&] {
        const auto genesis = headerOracle.LoadHeader(
            ot::blockchain::client::HeaderOracle::GenesisBlockHash(chain_));

        return genesis->as_Bitcoin();
    }();

    ASSERT_TRUE(previousHeader);

    for (auto i{0}; i < blocksToMine; ++i) {
        using OutputBuilder = ot::api::Factory::OutputBuilder;
        auto tx = miner_.Factory().BitcoinGenerationTransaction(
            chain_, previousHeader->Height() + 1, [&] {
                auto output = std::vector<OutputBuilder>{};
                const auto text = std::string{"null"};
                const auto keys = std::set<ot::api::client::blockchain::Key>{};
                output.emplace_back(
                    5000000000,
                    miner_.Factory().BitcoinScriptNullData(chain_, {text}),
                    keys);

                return output;
            }());

        ASSERT_TRUE(tx);

        auto block = miner_.Factory().BitcoinBlock(
            *previousHeader,
            tx,
            previousHeader->nBits(),
            {},
            previousHeader->Version(),
            [start{ot::Clock::now()}] {
                return (ot::Clock::now() - start) > std::chrono::minutes(1);
            });

        ASSERT_TRUE(block);

        const auto added = network.AddBlock(block);

        ASSERT_TRUE(added);

        previousHeader = block->Header().as_Bitcoin();

        ASSERT_TRUE(previousHeader);
    }

    constexpr auto limit = std::chrono::minutes(5);

    {
        ASSERT_EQ(blockFuture.wait_for(limit), std::future_status::ready);

        const auto [height, hash] = blockFuture.get();

        EXPECT_EQ(hash, previousHeader->Hash());
    }

    {
        ASSERT_EQ(walletFuture.wait_for(limit), std::future_status::ready);

        const auto height = walletFuture.get();

        EXPECT_EQ(height, blocksToMine);
    }
}

TEST_F(Regtest_fixture, shutdown) { Shutdown(); }
}  // namespace
