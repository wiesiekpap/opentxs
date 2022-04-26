// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/util/Log.hpp"
#include "ottest/fixtures/blockchain/Regtest.hpp"
#include "ottest/fixtures/blockchain/RegtestSimple.hpp"
#include "ottest/fixtures/paymentcode/VectorsV3.hpp"

namespace ottest
{

TEST_F(Regtest_fixture_simple, send_to_client)
{
    EXPECT_TRUE(Start());
    EXPECT_TRUE(Connect());

    const std::string name_alice = "Alice";
    const std::string name_bob = "Bob";

    Height target_height = 0, begin = 0;
    const auto blocks_number = 2;
    const auto coin_to_send = 100000;

    auto [user_alice, success_alice] = CreateClient(
        opentxs::Options{},
        3,
        name_alice,
        GetVectors3().alice_.words_,
        address_);
    EXPECT_TRUE(success_alice);

    auto [user_bob, success_bob] = CreateClient(
        opentxs::Options{}, 4, name_bob, GetVectors3().bob_.words_, address_);
    EXPECT_TRUE(success_bob);

    auto scan_listener_alice = std::make_unique<ScanListener>(*user_alice.api_);
    auto scan_listener_bob = std::make_unique<ScanListener>(*user_bob.api_);

    std::vector<std::reference_wrapper<const User>> users{user_alice, user_bob};
    MineBlocksForUsers(users, target_height, blocks_number);
    begin = target_height;
    target_height += static_cast<int>(MaturationInterval()) + 1;

    auto scan_listener_external_alice_f = scan_listener_alice->get_future(
        GetHDAccount(user_alice), bca::Subchain::External, target_height);
    auto scan_listener_internal_alice_f = scan_listener_alice->get_future(
        GetHDAccount(user_alice), bca::Subchain::Internal, target_height);
    auto scan_listener_external_bob_f = scan_listener_bob->get_future(
        GetHDAccount(user_bob), bca::Subchain::External, target_height);
    auto scan_listener_internal_bob_f = scan_listener_bob->get_future(
        GetHDAccount(user_bob), bca::Subchain::Internal, target_height);

    // mine MaturationInterval number block with
    MineBlocks(begin, static_cast<int>(MaturationInterval()) + 1);
    begin += MaturationInterval() + 1;

    EXPECT_TRUE(scan_listener_alice->wait(scan_listener_external_alice_f));
    EXPECT_TRUE(scan_listener_alice->wait(scan_listener_internal_alice_f));
    EXPECT_EQ(
        GetBalance(user_alice),
        amount_in_transaction_ * blocks_number * transaction_in_block_);

    EXPECT_TRUE(scan_listener_bob->wait(scan_listener_external_bob_f));
    EXPECT_TRUE(scan_listener_bob->wait(scan_listener_internal_bob_f));
    EXPECT_EQ(
        GetBalance(user_bob),
        amount_in_transaction_ * blocks_number * transaction_in_block_);

    const User* sender = &user_alice;
    const User* receiver = &user_bob;
    Amount sender_amount =
        amount_in_transaction_ * blocks_number * transaction_in_block_;
    Amount receiver_amount =
        amount_in_transaction_ * blocks_number * transaction_in_block_;

    const size_t numbers_of_test = 2;

    for (size_t number_of_test = 0; number_of_test < numbers_of_test;
         number_of_test++) {

        SendCoins(*receiver, *sender, target_height, coin_to_send);
        std::this_thread::sleep_for(std::chrono::seconds(20));
        
        auto loaded_transactions = CollectTransactionsForFeeCalculations(
            *sender, send_transactions_, transactions_);
        auto fee = CalculateFee(send_transactions_, loaded_transactions);
        send_transactions_.clear();

        EXPECT_EQ(GetBalance(*sender), sender_amount - coin_to_send - fee);
        EXPECT_EQ(GetBalance(*receiver), receiver_amount + coin_to_send);

        receiver_amount = GetBalance(*sender);
        sender_amount = GetBalance(*receiver);
        std::swap(sender, receiver);
    }

    std::this_thread::sleep_for(std::chrono::seconds(20));
    Shutdown();
}

}  // namespace ottest
