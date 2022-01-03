// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

namespace opentxs
{
class PasswordCallback;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT PasswordCaller
{
public:
    auto HaveCallback() const -> bool;

    void AskOnce(
        const PasswordPrompt& prompt,
        Secret& output,
        const std::string& key);
    void AskTwice(
        const PasswordPrompt& prompt,
        Secret& output,
        const std::string& key);
    void SetCallback(PasswordCallback* callback);

    PasswordCaller();

    ~PasswordCaller();

private:
    PasswordCallback* callback_;

    PasswordCaller(const PasswordCaller&) = delete;
    PasswordCaller(PasswordCaller&&) = delete;
    auto operator=(const PasswordCaller&) -> PasswordCaller& = delete;
    auto operator=(PasswordCaller&&) -> PasswordCaller& = delete;
};
}  // namespace opentxs
