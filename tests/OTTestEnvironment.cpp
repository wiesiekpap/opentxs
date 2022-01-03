// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"  // IWYU pragma: associated

#include "Basic.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/util/Options.hpp"

namespace ottest
{
OTTestEnvironment::OTTestEnvironment() noexcept
{
    auto& args = const_cast<ot::Options&>(ottest::Args(false));
    args.SetQtRootObject(GetQT());
}

auto OTTestEnvironment::SetUp() -> void { ot::InitContext(Args(false)); }

auto OTTestEnvironment::TearDown() -> void
{
    ot::Cleanup();
    WipeHome();
    StopQT();
}

OTTestEnvironment::~OTTestEnvironment() = default;
}  // namespace ottest
