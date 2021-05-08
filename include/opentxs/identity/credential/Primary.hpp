// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_CREDENTIAL_PRIMARY_HPP
#define OPENTXS_IDENTITY_CREDENTIAL_PRIMARY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/identity/credential/Key.hpp"

namespace opentxs
{
namespace proto
{
class HDPath;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace identity
{
namespace credential
{
class OPENTXS_EXPORT Primary : virtual public identity::credential::Key
{
public:
    OPENTXS_NO_EXPORT virtual bool Path(proto::HDPath& output) const = 0;
    virtual std::string Path() const = 0;

    ~Primary() override = default;

protected:
    Primary() noexcept {}  // TODO Signable

private:
    Primary(const Primary&) = delete;
    Primary(Primary&&) = delete;
    Primary& operator=(const Primary&) = delete;
    Primary& operator=(Primary&&) = delete;
};
}  // namespace credential
}  // namespace identity
}  // namespace opentxs
#endif
