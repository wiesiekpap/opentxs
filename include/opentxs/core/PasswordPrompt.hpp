// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP
#define OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Secret.hpp"

namespace opentxs
{
namespace api
{
namespace implementation
{
class Core;
}  // namespace implementation

class Core;
}  // namespace api

class Factory;
class PasswordPrompt;

using OTPasswordPrompt = Pimpl<PasswordPrompt>;
}  // namespace opentxs

namespace opentxs
{
/*
 PasswordPrompt
 This class is used for passing user data to the password callback.
 Whenever actually doing some OpenSSL call that involves a private key,
 just instantiate one of these and pass its address as the userdata for
 the OpenSSL call.  Then when the OT password callback is activated by
 OpenSSL, that pointer will be passed into the callback, so the user string
 can be displayed on the password dialog. (And also so the callback knows
 whether it was activated for a normal key or for a master key.) If it was
 activated for a normal key, then it will use the cached master key, or
 if that's timed out then it will try to decrypt a copy of it using the
 master Nym. Whereas if it WAS activated for the Master Nym, then it will
 just pop up the passphrase dialog and get his passphrase, and use that to
 decrypt the master key.

 NOTE: For internationalization later, we can add an PasswordPrompt constructor
 that takes a STRING CODE instead of an actual string. We can use an enum for
 this. Then we just pass the code there, instead of the string itself, and
 the class will do the work of looking up the actual string based on that code.
 */
class OPENTXS_EXPORT PasswordPrompt
{
public:
    auto GetDisplayString() const -> const char*;
    auto Password() const -> const Secret&;

    auto ClearPassword() -> bool;
    auto SetPassword(const Secret& password) -> bool;

    ~PasswordPrompt();

private:
    friend OTPasswordPrompt;
    friend opentxs::Factory;
    friend api::implementation::Core;

    auto clone() const noexcept -> PasswordPrompt*
    {
        return new PasswordPrompt(*this);
    }

    const api::Core& api_;
    std::string display_;
    OTSecret password_;

    PasswordPrompt(const api::Core& api, const std::string& display) noexcept;
    PasswordPrompt(const PasswordPrompt&) noexcept;
    PasswordPrompt(const PasswordPrompt&&) = delete;
    auto operator=(const PasswordPrompt&) -> const PasswordPrompt& = delete;
    auto operator=(const PasswordPrompt&&) -> const PasswordPrompt& = delete;
};
}  // namespace opentxs
#endif
