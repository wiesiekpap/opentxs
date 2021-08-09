// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <string>

#include "Basic.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/core/identifier/Server.hpp"

namespace ottest
{
std::string server_id_{};

TEST(ServerSession, create)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& server = otx.StartServer(0);
    server_id_ = server.ID().str();

    EXPECT_FALSE(server_id_.empty());

    ot::Cleanup();
}

TEST(ServerSession, restart)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& server = otx.StartServer(0);

    EXPECT_EQ(server_id_, server.ID().str());

    ot::Cleanup();
}
}  // namespace ottest
