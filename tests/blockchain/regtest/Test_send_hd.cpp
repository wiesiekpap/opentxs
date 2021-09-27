// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <string>
#include <utility>

#include "integration/Helpers.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/iterator/Bidirectional.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "rpc/Helpers.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
Counter account_list_{};
Counter account_activity_{};

TEST_F(Regtest_fixture_hd, init_opentxs) {}

TEST_F(Regtest_fixture_hd, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_hd, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_hd, init_ui_models)
{
    account_activity_.expected_ += 0;
    account_list_.expected_ += 1;
    init_account_activity(
        alice_, SendHD().Parent().AccountID(), account_activity_);
    init_account_list(alice_, account_list_);
}

TEST_F(Regtest_fixture_hd, account_activity_initial)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        0,
        0,
        u8"0 units",
        "",
        {},
        {test_chain_},
        0,
        {0, 0},
        {
            {"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn", true},
            {"2MzQwSSnBHWHqSAqtTVQ6v47XtaisrJa1Vc", true},
            {"2N2PJEucf6QY2kNFuJ4chQEBoyZWszRQE16", true},
            {"17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem", false},
            {"3EktnHQD7RiAE6uzMj2ZifT9YgRrkSgzQX", false},
            {"LM2WMpR1Rp6j3Sa59cMXMs1SPzj9eXpGc1", false},
            {"MTf4tP1TCNBn8dNkyxeBVoPrFCcVzxJvvh", false},
            {"QVk4MvUu7Wb7tZ1wvAeiUvdF7wxhvpyLLK", false},
            {"pS8EA1pKEVBvv3kGsSGH37R8YViBmuRCPn", false},
        },
        {},
        {},
    };

    ASSERT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_initial)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         0,
         0,
         u8"0 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, mine)
{
    auto future1 = listener_.get_future(SendHD(), Subchain::External, 1);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, 1);
    constexpr auto count{1};
    account_list_.expected_ += count;
    account_activity_.expected_ += (2 * count) + 1;

    EXPECT_TRUE(Mine(0, count, hd_generator_));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
}

TEST_F(Regtest_fixture_hd, first_block)
{
    const auto& blockchain =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto blockHash = blockchain.HeaderOracle().BestHash(1);

    ASSERT_FALSE(blockHash->empty());

    const auto pBlock = blockchain.BlockOracle().LoadBitcoin(blockHash).get();

    ASSERT_TRUE(pBlock);

    const auto& block = *pBlock;

    ASSERT_EQ(block.size(), 1);

    const auto pTx = block.at(0);

    ASSERT_TRUE(pTx);

    const auto& tx = *pTx;

    EXPECT_EQ(tx.ID(), transactions_.at(0));
    EXPECT_EQ(tx.BlockPosition(), 0);
    EXPECT_EQ(tx.Outputs().size(), 100);
}

TEST_F(Regtest_fixture_hd, account_activity_initial_receive)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        1,
        10000004950,
        u8"100.000\u202F049\u202F5 units",
        "",
        {},
        {test_chain_},
        100,
        {1, 1},
        {},
        {},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_initial_receive)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         10000004950,
         u8"100.000\u202F049\u202F5 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_inital_receive) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, spend)
{
    account_list_.expected_ += 1;
    account_activity_.expected_ += 2;
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto& widget = client_1_.UI().AccountActivity(
        alice_.nym_id_, SendHD().Parent().AccountID());
    constexpr auto sendAmount{u8"14 units"};
    constexpr auto address{"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn"};

    ASSERT_FALSE(widget.ValidateAmount(sendAmount).empty());
    ASSERT_TRUE(widget.ValidateAddress(address));

    auto future = network.SendToAddress(
        alice_.nym_id_, address, 1400000000, memo_outgoing_);
    const auto& txid = transactions_.emplace_back(future.get().second);

    EXPECT_FALSE(txid->empty());

    const auto& element = SendHD().BalanceElement(Subchain::Internal, 0);
    constexpr auto amount = ot::blockchain::Amount{99997807};
    expected_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(txid->Bytes(), 0),
        std::forward_as_tuple(
            element.PubkeyHash(), amount, Pattern::PayToPubkeyHash));
    const auto pTX = client_1_.Blockchain().LoadTransactionBitcoin(txid);

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    for (const auto& input : tx.Inputs()) {
        EXPECT_TRUE(txos_.SpendUnconfirmed(input.PreviousOutput()));
    }

    EXPECT_TRUE(txos_.AddUnconfirmed(tx, 0, SendHD()));
}

TEST_F(Regtest_fixture_hd, account_activity_after_unconfirmed_spend)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        1,
        8600002652,
        u8"86.000\u202F026\u202F52 units",
        "",
        {},
        {test_chain_},
        100,
        {1, 1},
        {},
        {},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1400002298,
                u8"-14.000\u202F022\u202F98 units",
                {},
                "",
                "",
                "Outgoing Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_after_unconfirmed_spend)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         8600002652,
         u8"86.000\u202F026\u202F52 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, wallet_after_unconfirmed_spend)
{
    EXPECT_TRUE(CheckTXODB());
}

TEST_F(Regtest_fixture_hd, confirm)
{
    auto future1 = listener_.get_future(SendHD(), Subchain::External, 2);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, 2);
    constexpr auto start{1};
    constexpr auto count{1};
    account_list_.expected_ += 0;
    account_activity_.expected_ += 1;
    const auto& txid = transactions_.at(1).get();
    const auto extra = [&] {
        auto output = std::vector<Transaction>{};
        const auto& pTX = output.emplace_back(
            client_1_.Blockchain().LoadTransactionBitcoin(txid));

        OT_ASSERT(pTX);

        return output;
    }();

    EXPECT_TRUE(Mine(start, count, default_, extra));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(start + count));
    EXPECT_TRUE(txos_.Confirm(transactions_.at(0)));
    EXPECT_TRUE(txos_.Confirm(txid));
}

TEST_F(Regtest_fixture_hd, account_activity_after_confirmed_spend)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        1,
        8600002652,
        u8"86.000\u202F026\u202F52 units",
        "",
        {},
        {test_chain_},
        100,
        {2, 2},
        {},
        {},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1400002298,
                u8"-14.000\u202F022\u202F98 units",
                {},
                "",
                "",
                "Outgoing Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_after_confirmed_spend)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         8600002652,
         u8"86.000\u202F026\u202F52 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, wallet_after_confirmed_spend)
{
    EXPECT_TRUE(CheckTXODB());
}

TEST_F(Regtest_fixture_hd, shutdown) { Shutdown(); }
}  // namespace ottest
