// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <boost/filesystem.hpp>
#include <cassert>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"

namespace fs = boost::filesystem;

auto OTTestEnvironment::Args() noexcept -> const ot::ArgList&
{
    static const auto out = ot::ArgList{
        {OPENTXS_ARG_HOME, {home()}},
        {OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}},
    };

    return out;
}

auto OTTestEnvironment::home() noexcept -> const std::string&
{
    static const auto output = [&] {
        const auto path = fs::temp_directory_path() /
                          fs::unique_path("opentxs-test-%%%%-%%%%-%%%%-%%%%");

        assert(fs::create_directories(path));

        return path.string();
    }();

    return output;
}

void OTTestEnvironment::SetUp() { ot::InitContext(Args()); }

void OTTestEnvironment::TearDown()
{
    ot::Cleanup();

    try {
        fs::remove_all(home());
    } catch (...) {
    }
}

OTTestEnvironment::~OTTestEnvironment() = default;
