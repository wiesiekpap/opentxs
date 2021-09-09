// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_CREDENTIAL_SECONDARY_HPP
#define OPENTXS_IDENTITY_CREDENTIAL_SECONDARY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/identity/credential/Key.hpp"

namespace opentxs
{
namespace identity
{
namespace credential
{
class OPENTXS_EXPORT Secondary : virtual public Key
{
public:
    ~Secondary() override = default;

protected:
    Secondary() noexcept {}  // TODO Signable

private:
    Secondary(const Secondary&) = delete;
    Secondary(Secondary&&) = delete;
    auto operator=(const Secondary&) -> Secondary& = delete;
    auto operator=(Secondary&&) -> Secondary& = delete;
};
}  // namespace credential
}  // namespace identity
}  // namespace opentxs
#endif
