// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <iostream>

#include "Cli.hpp"
#include "OTTestEnvironment.hpp"  // IWYU pragma: keep

int main(int argc, char** argv)
{
    auto parser = ArgumentParser{};
    parser.parse(
        argc, argv, const_cast<ot::ArgList&>(OTTestEnvironment::test_args_));

    if (parser.show_help_) {
        std::cout << parser.options() << "\n";

        return 0;
    }

    ::testing::AddGlobalTestEnvironment(new OTTestEnvironment());
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
