// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTCALLER_HPP
#define OPENTXS_CORE_CRYPTO_OTCALLER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

namespace opentxs
{
class OTCallback;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs
{
class OTCaller
{
public:
    OPENTXS_EXPORT bool HaveCallback() const;

    OPENTXS_EXPORT void AskOnce(
        const PasswordPrompt& prompt,
        Secret& output,
        const std::string& key);
    OPENTXS_EXPORT void AskTwice(
        const PasswordPrompt& prompt,
        Secret& output,
        const std::string& key);
    OPENTXS_EXPORT void SetCallback(OTCallback* callback);

    OPENTXS_EXPORT OTCaller();

    OPENTXS_EXPORT ~OTCaller();

private:
    OTCallback* callback_{nullptr};

    OTCaller(const OTCaller&) = delete;
    OTCaller(OTCaller&&) = delete;
    OTCaller& operator=(const OTCaller&) = delete;
    OTCaller& operator=(OTCaller&&) = delete;
};
}  // namespace opentxs
#endif
