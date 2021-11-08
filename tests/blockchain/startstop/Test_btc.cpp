// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>

#include "Helpers.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace ottest
{
TEST_F(Test_StartStop, init_opentxs) {}

TEST_F(Test_StartStop, btc)
{
    EXPECT_TRUE(
        api_.Network().Blockchain().Start(b::Type::Bitcoin, "127.0.0.2"));
    EXPECT_TRUE(api_.Network().Blockchain().Stop(b::Type::Bitcoin));
}
}  // namespace ottest
