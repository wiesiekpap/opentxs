// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_IDENTIFIER_SERVER_HPP
#define OPENTXS_CORE_IDENTIFIER_SERVER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Identifier.hpp"

namespace opentxs
{
namespace identifier
{
class Server;
}  // namespace identifier

using OTServerID = Pimpl<identifier::Server>;

OPENTXS_EXPORT auto operator==(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator!=(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator<=(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
OPENTXS_EXPORT auto operator>=(
    const OTServerID& lhs,
    const opentxs::Identifier& rhs) noexcept -> bool;
}  // namespace opentxs

namespace opentxs
{
namespace identifier
{
class OPENTXS_EXPORT Server : virtual public opentxs::Identifier
{
public:
    static auto Factory() -> OTServerID;
    static auto Factory(const std::string& rhs) -> OTServerID;
    static auto Factory(const String& rhs) -> OTServerID;

    ~Server() override = default;

protected:
    Server() = default;

private:
    friend OTServerID;

#ifndef _WIN32
    auto clone() const -> Server* override = 0;
#endif
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace identifier
}  // namespace opentxs
#endif
