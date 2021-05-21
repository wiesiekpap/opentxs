// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <string>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"

namespace ot = opentxs;

class OTTestEnvironment : public testing::Environment
{
public:
    static auto Args() noexcept -> const ot::ArgList&;

    void SetUp() override;
    void TearDown() override;

    ~OTTestEnvironment() override;

private:
    static auto home() noexcept -> const std::string&;
};
