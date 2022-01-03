// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <utility>

#include "1_Internal.hpp"  // IWYU pragma: keep
#include "integration/Helpers.hpp"
#include "internal/network/p2p/Factory.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/ProtocolVersion.hpp"
#include "opentxs/core/identifier/Type.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/p2p/Acknowledgement.hpp"
#include "opentxs/network/p2p/Base.hpp"
#include "opentxs/network/p2p/Data.hpp"
#include "opentxs/network/p2p/MessageType.hpp"
#include "opentxs/network/p2p/PublishContract.hpp"
#include "opentxs/network/p2p/PublishContractReply.hpp"
#include "opentxs/network/p2p/Query.hpp"
#include "opentxs/network/p2p/QueryContract.hpp"
#include "opentxs/network/p2p/QueryContractReply.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace ottest
{
TEST_F(Regtest_fixture_sync, init_opentxs) {}

TEST_F(Regtest_fixture_sync, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_sync, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_sync, sync_genesis)
{
    sync_req_.expected_ += 2;
    const auto& chain = client_2_.Network().Blockchain().GetChain(test_chain_);
    const auto pos = chain.HeaderOracle().BestChain();

    EXPECT_TRUE(sync_req_.request(pos));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
    const auto& chain = client_2_.Network().Blockchain().GetChain(test_chain_);
    const auto genesis = Position{0, chain.HeaderOracle().BestHash(0)};

    EXPECT_TRUE(sync_req_.request(genesis));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

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
    const auto original = opentxs::factory::BlockchainSyncQuery(0);

    EXPECT_EQ(original.Type(), otsync::MessageType::query);
    EXPECT_NE(original.Version(), 0);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 2);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 1);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQuery();

        EXPECT_EQ(query.Type(), otsync::MessageType::query);
        EXPECT_NE(query.Version(), 0);
    }

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};
            out.AddFrame("Header frame 1");
            out.AddFrame("Header frame 2");
            out.StartBody();

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 4);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 2);
        EXPECT_EQ(body.size(), 1);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQuery();

        EXPECT_EQ(query.Type(), otsync::MessageType::query);
        EXPECT_NE(query.Version(), 0);
    }
}

TEST_F(Regtest_fixture_sync, make_contracts)
{
    ASSERT_TRUE(alex_.nym_);
    ASSERT_FALSE(alex_.nym_id_->empty());
    ASSERT_FALSE(notary_.has_value());
    ASSERT_FALSE(unit_.has_value());

    const auto reason = client_2_.Factory().PasswordPrompt(__func__);
    notary_.emplace(client_2_.Wallet().Server(
        alex_.nym_id_->str(),
        "Example notary",
        "Don't use",
        {{ot::core::AddressType::Inproc,
          ot::contract::ProtocolVersion::Legacy,
          "inproc://lol_nope",
          80,
          2}},
        reason,
        2));

    ASSERT_TRUE(notary_.has_value());
    ASSERT_FALSE(notary_.value()->ID()->empty());

    unit_.emplace(client_2_.Wallet().CurrencyContract(
        alex_.nym_id_->str(),
        "My Dollars",
        "Example only",
        ot::core::UnitType::USD,
        ot::unsigned_amount(0, 1, 100),
        reason));

    ASSERT_TRUE(unit_.has_value());
    ASSERT_FALSE(unit_.value()->ID()->empty());
}

TEST_F(Regtest_fixture_sync, query_nonexistent_nym)
{
    ASSERT_TRUE(alex_.nym_);

    const auto& id = alex_.nym_id_.get();

    EXPECT_EQ(id.Type(), ot::identifier::Type::nym);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), otsync::MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::nym);
        EXPECT_FALSE(opentxs::valid(reply.Payload()));
    }
}

TEST_F(Regtest_fixture_sync, query_nonexistent_notary)
{
    ASSERT_TRUE(notary_.has_value());

    const auto id = notary_.value()->ID();

    EXPECT_EQ(id->Type(), ot::identifier::Type::notary);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), otsync::MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::notary);
        EXPECT_FALSE(opentxs::valid(reply.Payload()));
    }
}

TEST_F(Regtest_fixture_sync, query_nonexistent_unit)
{
    ASSERT_TRUE(unit_.has_value());

    const auto id = unit_.value()->ID();

    EXPECT_EQ(id->Type(), ot::identifier::Type::unitdefinition);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), otsync::MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::unit);
        EXPECT_FALSE(opentxs::valid(reply.Payload()));
    }
}

TEST_F(Regtest_fixture_sync, publish_nym)
{
    ASSERT_TRUE(alex_.nym_);

    const auto& id = alex_.nym_id_.get();
    const auto original =
        opentxs::factory::BlockchainSyncPublishContract(*alex_.nym_);

    EXPECT_EQ(original.Type(), otsync::MessageType::publish_contract);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);
    EXPECT_EQ(original.ContractType(), ot::contract::Type::nym);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::publish_contract);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asPublishContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::publish_contract);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
        EXPECT_EQ(query.ContractType(), ot::contract::Type::nym);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::publish_ack);

        const auto& reply = base->asPublishContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync, publish_notary)
{
    ASSERT_TRUE(notary_.has_value());

    const auto id = notary_.value()->ID();
    const auto original =
        opentxs::factory::BlockchainSyncPublishContract(notary_.value());

    EXPECT_EQ(original.Type(), otsync::MessageType::publish_contract);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);
    EXPECT_EQ(original.ContractType(), ot::contract::Type::notary);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::publish_contract);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asPublishContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::publish_contract);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
        EXPECT_EQ(query.ContractType(), ot::contract::Type::notary);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::publish_ack);

        const auto& reply = base->asPublishContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync, publish_unit)
{
    ASSERT_TRUE(unit_.has_value());

    const auto id = unit_.value()->ID();
    const auto original =
        opentxs::factory::BlockchainSyncPublishContract(unit_.value());

    EXPECT_EQ(original.Type(), otsync::MessageType::publish_contract);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);
    EXPECT_EQ(original.ContractType(), ot::contract::Type::unit);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 5);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 4);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::publish_contract);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asPublishContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::publish_contract);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
        EXPECT_EQ(query.ContractType(), ot::contract::Type::unit);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::publish_ack);

        const auto& reply = base->asPublishContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_TRUE(reply.Success());
    }
}

TEST_F(Regtest_fixture_sync, query_nym)
{
    ASSERT_TRUE(alex_.nym_);

    const auto& id = alex_.nym_id_.get();
    const auto expected = [&] {
        auto out = client_2_.Factory().Data();
        alex_.nym_->Serialize(out->WriteInto());

        return out;
    }();

    EXPECT_EQ(id.Type(), ot::identifier::Type::nym);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), otsync::MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::nym);
        ASSERT_TRUE(opentxs::valid(reply.Payload()));
        EXPECT_EQ(expected->Bytes(), reply.Payload());
    }
}

TEST_F(Regtest_fixture_sync, query_notary)
{
    ASSERT_TRUE(notary_.has_value());

    const auto id = notary_.value()->ID();
    const auto expected = [&] {
        auto out = client_2_.Factory().Data();
        notary_.value()->Serialize(out->WriteInto());

        return out;
    }();

    EXPECT_EQ(id->Type(), ot::identifier::Type::notary);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), otsync::MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::notary);
        ASSERT_TRUE(opentxs::valid(reply.Payload()));
        EXPECT_EQ(expected->Bytes(), reply.Payload());
    }
}

TEST_F(Regtest_fixture_sync, query_unit)
{
    ASSERT_TRUE(unit_.has_value());

    const auto id = unit_.value()->ID();
    const auto expected = [&] {
        auto out = client_2_.Factory().Data();
        unit_.value()->Serialize(out->WriteInto());

        return out;
    }();

    EXPECT_EQ(id->Type(), ot::identifier::Type::unitdefinition);

    const auto original = opentxs::factory::BlockchainSyncQueryContract(id);

    EXPECT_EQ(original.Type(), otsync::MessageType::contract_query);
    EXPECT_NE(original.Version(), 0);
    EXPECT_EQ(original.ID(), id);

    {
        const auto serialized = [&] {
            auto out = opentxs::network::zeromq::Message{};

            EXPECT_TRUE(original.Serialize(out));

            return out;
        }();

        EXPECT_EQ(serialized.size(), 3);

        const auto header = serialized.Header();
        const auto body = serialized.Body();

        EXPECT_EQ(header.size(), 0);
        EXPECT_EQ(body.size(), 2);

        auto recovered = client_2_.Factory().BlockchainSyncMessage(serialized);

        ASSERT_TRUE(recovered);

        EXPECT_EQ(recovered->Type(), otsync::MessageType::contract_query);
        EXPECT_NE(recovered->Version(), 0);

        const auto& query = recovered->asQueryContract();

        EXPECT_EQ(query.Type(), otsync::MessageType::contract_query);
        EXPECT_NE(query.Version(), 0);
        EXPECT_EQ(query.ID(), id);
    }

    sync_req_.expected_ += 1;

    EXPECT_TRUE(sync_req_.request(original));
    ASSERT_TRUE(sync_req_.wait());

    {
        const auto& msg = sync_req_.get(++sync_req_.checked_);
        const auto base = client_2_.Factory().BlockchainSyncMessage(msg);

        ASSERT_TRUE(base);
        ASSERT_EQ(base->Type(), otsync::MessageType::contract);

        const auto& reply = base->asQueryContractReply();

        EXPECT_EQ(reply.ID(), id);
        EXPECT_EQ(reply.ContractType(), ot::contract::Type::unit);
        ASSERT_TRUE(opentxs::valid(reply.Payload()));
        EXPECT_EQ(expected->Bytes(), reply.Payload());
    }
}

TEST_F(Regtest_fixture_sync, shutdown) { Shutdown(); }
}  // namespace ottest
