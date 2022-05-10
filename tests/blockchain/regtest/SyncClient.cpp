// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Regtest.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "ottest/fixtures/blockchain/RegtestSimple.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/paymentcode/VectorsV3.hpp"

namespace ottest
{

TEST_F(Regtest_fixture_simple, start_stop_client)
{
    EXPECT_TRUE(Start());
    EXPECT_TRUE(Connect());

    const int numbers_of_test = 2;
    const auto blocks_mine_for_alice = 2;
    bool mine_only_in_first_test = true;
    Height begin = 0;
    auto expected_balance = 0;

    for (int number_of_test = 0; number_of_test < numbers_of_test;
         number_of_test++) {
        ot::LogConsole()(
            "Start test number: " + std::to_string(number_of_test + 1))
            .Flush();

        auto [user, success] = CreateClient(
            opentxs::Options{},
            number_of_test + 3,
            core_wallet_.name_,
            GetVectors3().alice_.words_,
            address_);
        EXPECT_TRUE(success);

        WaitForSynchro(user, target_height, expected_balance);
        if (!mine_only_in_first_test || number_of_test == 0) {
            target_height += static_cast<Height>(blocks_mine_for_alice);

            auto scan_listener = std::make_unique<ScanListener>(*user.api_);

            auto scan_listener_external_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::External, target_height);
            auto scan_listener_internal_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::Internal, target_height);

            // mine coin for Alice
            auto mined_header = MineBlocks(
                user,
                begin,
                blocks_mine_for_alice,
                transaction_in_block_,
                amount_in_transaction_);

            EXPECT_TRUE(scan_listener->wait(scan_listener_external_f));
            EXPECT_TRUE(scan_listener->wait(scan_listener_internal_f));

            begin += blocks_mine_for_alice;
            auto count = static_cast<int>(MaturationInterval());
            target_height += count;

            scan_listener_external_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::External, target_height);
            scan_listener_internal_f = scan_listener->get_future(
                GetHDAccount(user), bca::Subchain::Internal, target_height);

            // mine MaturationInterval number block with
            MineBlocks(begin, count);
            begin += count;

            EXPECT_TRUE(scan_listener->wait(scan_listener_external_f));
            EXPECT_TRUE(scan_listener->wait(scan_listener_internal_f));

            expected_balance += amount_in_transaction_ * blocks_mine_for_alice *
                                transaction_in_block_;
            WaitForSynchro(user, target_height, expected_balance);
        }

        EXPECT_EQ(GetBalance(user), expected_balance);

        CloseClient(core_wallet_.name_);
    }

    Shutdown();
}

}  // namespace ottest
