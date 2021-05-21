// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/filesystem.hpp>
#include <cassert>

#include "OTLowLevelTestEnvironment.hpp"
#include "opentxs/OT.hpp"

namespace fs = boost::filesystem;

auto OTLowLevelTestEnvironment::Args() noexcept -> const ot::ArgList&
{
    static const auto out = ot::ArgList{
        {OPENTXS_ARG_HOME, {home()}},
    };

    return out;
}

auto OTLowLevelTestEnvironment::home() noexcept -> const std::string&
{
    static const auto output = [&] {
        const auto path = fs::temp_directory_path() /
                          fs::unique_path("opentxs-test-%%%%-%%%%-%%%%-%%%%");

        assert(fs::create_directories(path));

        return path.string();
    }();

    return output;
}

void OTLowLevelTestEnvironment::SetUp() {}

void OTLowLevelTestEnvironment::TearDown()
{
    ot::Cleanup();

    try {
        fs::remove_all(home());
    } catch (...) {
    }
}

OTLowLevelTestEnvironment::~OTLowLevelTestEnvironment() = default;
