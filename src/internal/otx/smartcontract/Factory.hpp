// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

namespace opentxs
{
class OTScript;
}  // namespace opentxs

namespace opentxs::factory
{
auto OTScript(const std::string& script_type = "")
    -> std::shared_ptr<opentxs::OTScript>;
auto OTScript(
    const std::string& script_type,
    const std::string& script_contents) -> std::shared_ptr<opentxs::OTScript>;
auto OTScriptChai() -> std::shared_ptr<opentxs::OTScript>;
auto OTScriptChai(const std::string& script_contents)
    -> std::shared_ptr<opentxs::OTScript>;
}  // namespace opentxs::factory
