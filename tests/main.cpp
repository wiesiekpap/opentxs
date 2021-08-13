// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>
#include <string>

#include "Basic.hpp"
#include "Cli.hpp"
#include "OTTestEnvironment.hpp"
#include "opentxs/api/Options.hpp"

int main(int argc, char** argv)
{
    auto& args = const_cast<ot::Options&>(ottest::Args(false));
    args.ParseCommandLine(argc, argv);
    auto parser = ottest::ArgumentParser{};
    parser.parse(argc, argv);

    if (parser.show_help_) {
        std::cout << parser.help(args) << "\n";

        return 0;
    }

    ::testing::AddGlobalTestEnvironment(new ottest::OTTestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
