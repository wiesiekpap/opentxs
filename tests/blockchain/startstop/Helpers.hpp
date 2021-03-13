// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "1_Internal.hpp"
#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"

namespace b = ot::blockchain;

namespace
{
class Test_StartStop : public ::testing::Test
{
public:
    const ot::api::client::Manager& api_;

    Test_StartStop()
        : api_(ot::Context().StartClient(OTTestEnvironment::test_args_, 0))
    {
    }
};
}  // namespace
