// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP
#define OPENTXS_CORE_CRYPTO_OTPASSWORDDATA_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <memory>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Secret.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::PasswordPrompt::api;
%ignore opentxs::PasswordPrompt::Password;
// clang-format on
#endif

namespace opentxs
{
namespace api
{
namespace implementation
{
class Core;
}  // namespace implementation

namespace internal
{
struct Core;
}  // namespace internal
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
class PasswordPrompt
{
public:
    OPENTXS_EXPORT const char* GetDisplayString() const;
    OPENTXS_EXPORT const Secret& Password() const;

    OPENTXS_EXPORT bool ClearPassword();
    OPENTXS_EXPORT bool SetPassword(const Secret& password);

    OPENTXS_EXPORT ~PasswordPrompt();

private:
    friend OTPasswordPrompt;
    friend opentxs::Factory;
    friend api::implementation::Core;

    PasswordPrompt* clone() const noexcept { return new PasswordPrompt(*this); }

    const api::internal::Core& api_;
    std::string display_;
    OTSecret password_;

    PasswordPrompt(
        const api::internal::Core& api,
        const std::string& display) noexcept;
    PasswordPrompt(const PasswordPrompt&) noexcept;
    PasswordPrompt(const PasswordPrompt&&) = delete;
    const PasswordPrompt& operator=(const PasswordPrompt&) = delete;
    const PasswordPrompt& operator=(const PasswordPrompt&&) = delete;
};
}  // namespace opentxs
#endif
