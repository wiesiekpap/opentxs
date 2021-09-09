// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP
#define OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#if OT_SCRIPT_CHAI
#include <cstddef>
#include <string>

#include "opentxs/Version.hpp"
#include "opentxs/core/script/OTScript.hpp"

namespace chaiscript
{
class ChaiScript;
}  // namespace chaiscript

namespace opentxs
{
class OTVariable;
class String;

class OPENTXS_EXPORT OTScriptChai final : public OTScript
{
public:
    OTScriptChai();
    OTScriptChai(const String& strValue);
    OTScriptChai(const char* new_string);
    OTScriptChai(const char* new_string, size_t sizeLength);
    OTScriptChai(const std::string& new_string);

    ~OTScriptChai() final;

    auto ExecuteScript(OTVariable* pReturnVar = nullptr) -> bool final;
    chaiscript::ChaiScript* const chai_{nullptr};

private:
    OTScriptChai(const OTScriptChai&) = delete;
    OTScriptChai(OTScriptChai&&) = delete;
    auto operator=(const OTScriptChai&) -> OTScriptChai& = delete;
    auto operator=(OTScriptChai&&) -> OTScriptChai& = delete;
};
}  // namespace opentxs
#endif  // OT_SCRIPT_CHAI
#endif
