// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_SERVER_HPP
#define OPENTXS_CORE_IDENTIFIER_SERVER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::identifier::Server::Factory;
%extend opentxs::identifier::Server {
    static OTServerID Factory()
    {
        return opentxs::identifier::Server::Factory();
    }
    static OTServerID Factory(
        const std::string& rhs)
    {
        return opentxs::identifier::Server::Factory(rhs);
    }
}
%rename (ServerID) opentxs::identifier::Server;
%template(OTServerID) opentxs::Pimpl<opentxs::identifier::Server>;
// clang-format on
#endif

namespace opentxs
{
namespace identifier
{
class Server;
}  // namespace identifier

using OTServerID = Pimpl<identifier::Server>;

#ifndef SWIG
OPENTXS_EXPORT bool operator==(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator!=(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator<(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator>(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator<=(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept;
OPENTXS_EXPORT bool operator>=(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept;
#endif
}  // namespace opentxs

namespace opentxs
{
namespace identifier
{
class Server : virtual public opentxs::Identifier
{
public:
#ifndef SWIG
    OPENTXS_EXPORT static OTServerID Factory();
    OPENTXS_EXPORT static OTServerID Factory(const std::string& rhs);
    OPENTXS_EXPORT static OTServerID Factory(const String& rhs);
#endif

    OPENTXS_EXPORT ~Server() override = default;

protected:
    Server() = default;

private:
    friend OTServerID;

#ifndef _WIN32
    Server* clone() const override = 0;
#endif
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    Server& operator=(const Server&) = delete;
    Server& operator=(Server&&) = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
