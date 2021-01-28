// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"

namespace ot = opentxs;

class OTLowLevelTestEnvironment : public testing::Environment
{
public:
    static const ot::ArgList test_args_;

    void SetUp() override;
    void TearDown() override;

    ~OTLowLevelTestEnvironment() override;

private:
    static std::string random_path();
};
