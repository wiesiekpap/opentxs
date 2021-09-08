// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "OTTestEnvironment.hpp"  // IWYU pragma: associated

#include "Basic.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Options.hpp"

namespace ottest
{
OTTestEnvironment::OTTestEnvironment() noexcept
    : qt_event_loop_(StartQT())
{
    auto& args = const_cast<ot::Options&>(ottest::Args(false));
    args.SetQtRootObject(GetQT());
}

auto OTTestEnvironment::SetUp() -> void { ot::InitContext(Args(false)); }

auto OTTestEnvironment::TearDown() -> void
{
    ot::Cleanup();
    // NOTE ideally StopQT() should be called in ~OTTestEnvironment() instead of
    // in this function but doing that causes a segfault. It probably has
    // something to do with Qt's thread affinity rules but who knows really.
    StopQT();
    WipeHome();
}

OTTestEnvironment::~OTTestEnvironment()
{
    // StopQT(); NOTE: see OTTestEnvironment::TearDown()

    if (qt_event_loop_.joinable()) { qt_event_loop_.join(); }
}
}  // namespace ottest
