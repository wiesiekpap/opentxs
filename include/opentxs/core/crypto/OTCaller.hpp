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
class OPENTXS_EXPORT OTCaller
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
    void SetCallback(OTCallback* callback);

    OTCaller();

    ~OTCaller();

private:
    OTCallback* callback_{nullptr};

    OTCaller(const OTCaller&) = delete;
    OTCaller(OTCaller&&) = delete;
    auto operator=(const OTCaller&) -> OTCaller& = delete;
    auto operator=(OTCaller&&) -> OTCaller& = delete;
};
}  // namespace opentxs
#endif
