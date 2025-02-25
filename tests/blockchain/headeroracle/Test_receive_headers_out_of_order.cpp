// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/fixtures/blockchain/HeaderOracle.hpp"

namespace ottest
{
TEST_F(Test_HeaderOracle, receive_headers_out_of_order)
{
    EXPECT_TRUE(create_blocks(create_5_));
    EXPECT_TRUE(apply_blocks(sequence_5_));
    EXPECT_TRUE(verify_post_state(post_state_5_));
    EXPECT_TRUE(verify_siblings(siblings_5_));

    {
        const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{
            BLOCK_3,
            BLOCK_12,
            BLOCK_4,
            BLOCK_5,
            BLOCK_6,
            BLOCK_13,
            BLOCK_14,
        };

        EXPECT_TRUE(verify_hash_sequence(3, 0, expected));
        EXPECT_TRUE(verify_hash_sequence(3, 8, expected));
        EXPECT_TRUE(verify_hash_sequence(3, 7, expected));
        EXPECT_FALSE(verify_hash_sequence(3, 6, expected));
    }

    {
        const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{
            BLOCK_12,
            BLOCK_4,
            BLOCK_5,
        };

        EXPECT_TRUE(verify_hash_sequence(4, 3, expected));
    }

    {
        const auto expected = ot::UnallocatedVector<ot::UnallocatedCString>{
            BLOCK_12,
            BLOCK_5,
            BLOCK_4,
        };

        EXPECT_FALSE(verify_hash_sequence(4, 3, expected));
    }

    {
        const auto existing =
            header_oracle_.LoadHeader(get_block_hash(BLOCK_8));

        ASSERT_TRUE(existing);

        {
            const auto data = header_oracle_.BestChain(existing->Position());

            ASSERT_EQ(data.size(), 5);
            EXPECT_EQ(data.at(0).hash_, get_block_hash(BLOCK_4));
            EXPECT_EQ(data.at(1).hash_, get_block_hash(BLOCK_5));
            EXPECT_EQ(data.at(2).hash_, get_block_hash(BLOCK_6));
            EXPECT_EQ(data.at(3).hash_, get_block_hash(BLOCK_13));
            EXPECT_EQ(data.at(4).hash_, get_block_hash(BLOCK_14));
        }
        {
            const auto target =
                header_oracle_.LoadHeader(get_block_hash(BLOCK_11));

            ASSERT_TRUE(target);

            const auto data = header_oracle_.Ancestors(
                existing->Position(), target->Position());

            ASSERT_EQ(data.size(), 3);
            EXPECT_EQ(data.at(0).hash_, get_block_hash(BLOCK_3));
            EXPECT_EQ(data.at(1).hash_, get_block_hash(BLOCK_10));
            EXPECT_EQ(data.at(2).hash_, get_block_hash(BLOCK_11));
        }
        {
            const auto target =
                header_oracle_.LoadHeader(get_block_hash(BLOCK_5));

            ASSERT_TRUE(target);

            const auto data = header_oracle_.Ancestors(
                existing->Position(), target->Position());

            ASSERT_EQ(data.size(), 2);
            EXPECT_EQ(data.at(0).hash_, get_block_hash(BLOCK_4));
            EXPECT_EQ(data.at(1).hash_, get_block_hash(BLOCK_5));
        }
    }
}
}  // namespace ottest
