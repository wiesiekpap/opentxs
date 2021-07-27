// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Basic.hpp"  // IWYU pragma: associated

#include <boost/filesystem.hpp>
#include <cassert>
#include <map>
#include <set>

namespace fs = boost::filesystem;

namespace ottest
{
auto Args(bool lowlevel) noexcept -> const ot::ArgList&
{
    static const auto minimal = ot::ArgList{
        {OPENTXS_ARG_HOME, {Home()}},
    };
    static const auto full = [&] {
        auto out{minimal};
        out[OPENTXS_ARG_STORAGE_PLUGIN].emplace("mem");

        return out;
    }();

    if (lowlevel) {

        return minimal;
    } else {

        return full;
    }
}

auto Home() noexcept -> const std::string&
{
    static const auto output = [&] {
        const auto path = fs::temp_directory_path() /
                          fs::unique_path("opentxs-test-%%%%-%%%%-%%%%-%%%%");

        assert(fs::create_directories(path));

        return path.string();
    }();

    return output;
}

auto WipeHome() noexcept -> void
{
    try {
        fs::remove_all(Home());
    } catch (...) {
    }
}
}  // namespace ottest
