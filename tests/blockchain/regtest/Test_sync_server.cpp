// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"
#include "opentxs/network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/Data.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/Query.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_sync, init_opentxs) {}

TEST_F(Regtest_fixture_sync, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_sync, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_sync, sync_genesis)
{
    sync_req_.expected_ += 2;
    const auto& chain = client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto pos = chain.HeaderOracle().BestChain();

    EXPECT_TRUE(sync_req_.request(pos));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(sync_req_.check(states.front(), pos));
        EXPECT_EQ(ack.Endpoint(), sync_server_update_public_);
    }
    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(sync_req_.check(state, pos));
        EXPECT_EQ(blocks.size(), 0);
    }
}

TEST_F(Regtest_fixture_sync, mine)
{
    constexpr auto count{10};
    sync_sub_.expected_ += count;

    EXPECT_TRUE(Mine(0, count));
    EXPECT_TRUE(sync_sub_.wait());

    const auto& chain = client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto best = chain.HeaderOracle().BestChain();

    EXPECT_EQ(best.first, 10);
    EXPECT_EQ(best.second, mined_blocks_.get(9).get());
}

TEST_F(Regtest_fixture_sync, sync_full)
{
    sync_req_.expected_ += 2;
    const auto& chain = client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto genesis = Position{0, chain.HeaderOracle().BestHash(0)};

    EXPECT_TRUE(sync_req_.request(genesis));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(sync_req_.check(states.front(), 9));
        EXPECT_EQ(ack.Endpoint(), sync_server_update_public_);
    }
    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(sync_req_.check(state, 9));
        ASSERT_EQ(blocks.size(), 10);
        EXPECT_TRUE(sync_req_.check(blocks.at(0), 0));
        EXPECT_TRUE(sync_req_.check(blocks.at(1), 1));
        EXPECT_TRUE(sync_req_.check(blocks.at(2), 2));
        EXPECT_TRUE(sync_req_.check(blocks.at(3), 3));
        EXPECT_TRUE(sync_req_.check(blocks.at(4), 4));
        EXPECT_TRUE(sync_req_.check(blocks.at(5), 5));
        EXPECT_TRUE(sync_req_.check(blocks.at(6), 6));
        EXPECT_TRUE(sync_req_.check(blocks.at(7), 7));
        EXPECT_TRUE(sync_req_.check(blocks.at(8), 8));
        EXPECT_TRUE(sync_req_.check(blocks.at(9), 9));
    }
}

TEST_F(Regtest_fixture_sync, sync_partial)
{
    sync_req_.expected_ += 2;
    const auto start = Position{6, mined_blocks_.get(6).get()};

    EXPECT_TRUE(sync_req_.request(start));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(sync_req_.check(states.front(), 9));
        EXPECT_EQ(ack.Endpoint(), sync_server_update_public_);
    }
    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(sync_req_.check(state, 9));
        ASSERT_EQ(blocks.size(), 4);
        EXPECT_TRUE(sync_req_.check(blocks.at(0), 6));
        EXPECT_TRUE(sync_req_.check(blocks.at(1), 7));
        EXPECT_TRUE(sync_req_.check(blocks.at(2), 8));
        EXPECT_TRUE(sync_req_.check(blocks.at(3), 9));
    }
}

TEST_F(Regtest_fixture_sync, reorg)
{
    constexpr auto count{4};
    sync_sub_.expected_ += count;

    EXPECT_TRUE(Mine(8, count));
    EXPECT_TRUE(sync_sub_.wait());

    const auto& chain = client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto best = chain.HeaderOracle().BestChain();

    EXPECT_EQ(best.first, 12);
    EXPECT_EQ(best.second, mined_blocks_.get(13).get());
}

TEST_F(Regtest_fixture_sync, sync_reorg)
{
    sync_req_.expected_ += 2;
    const auto start = Position{10, mined_blocks_.get(9).get()};

    EXPECT_TRUE(sync_req_.request(start));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_ack);

        const auto& ack = base->asAcknowledgement();
        const auto& states = ack.State();

        ASSERT_EQ(states.size(), 1);
        EXPECT_TRUE(sync_req_.check(states.front(), 13));
        EXPECT_EQ(ack.Endpoint(), sync_server_update_public_);
    }
    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = otsync::Factory(client_1_, msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::sync_reply);

        const auto& reply = base->asData();
        const auto& state = reply.State();
        const auto& blocks = reply.Blocks();

        EXPECT_TRUE(sync_req_.check(state, 13));
        ASSERT_EQ(blocks.size(), 4);
        EXPECT_TRUE(sync_req_.check(blocks.at(0), 10));
        EXPECT_TRUE(sync_req_.check(blocks.at(1), 11));
        EXPECT_TRUE(sync_req_.check(blocks.at(2), 12));
        EXPECT_TRUE(sync_req_.check(blocks.at(3), 13));
    }
}

TEST_F(Regtest_fixture_sync, query)
{
    const auto original = otsync::Query{0};

    EXPECT_EQ(original.Type(), otsync::MessageType::query);
    EXPECT_NE(original.Version(), 0);

    {
        const auto serialized = [&] {
            auto out = client_1_.Network().ZeroMQ().Message();

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized->size(), 2);

        const auto header = serialized->Header();
        const auto body = serialized->Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 1);

        auto recovered = otsync::Factory(client_1_, serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQuery();

        EXPECT_EQ(query.Type(), otsync::MessageType::query);
        EXPECT_NE(query.Version(), 0);
    }

    {
        const auto serialized = [&] {
            auto out = client_1_.Network().ZeroMQ().Message();
            out->AddFrame("Header frame 1");
            out->AddFrame("Header frame 2");
            out->AddFrame();

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized->size(), 4);

        const auto header = serialized->Header();
        const auto body = serialized->Body();

        EXPECT_EQ(header.size(), 2);
        EXPECT_EQ(body.size(), 1);

        auto recovered = otsync::Factory(client_1_, serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQuery();

        EXPECT_EQ(query.Type(), otsync::MessageType::query);
        EXPECT_NE(query.Version(), 0);
    }
}

TEST_F(Regtest_fixture_sync, shutdown) { Shutdown(); }
}  // namespace ottest
