// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYCONSENSUS_HPP
#define OPENTXS_PROTOBUF_VERIFYCONSENSUS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <set>

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
OPENTXS_EXPORT const VersionMap& ContextAllowedServer() noexcept;
OPENTXS_EXPORT const VersionMap& ContextAllowedClient() noexcept;
OPENTXS_EXPORT const VersionMap& ContextAllowedSignature() noexcept;
OPENTXS_EXPORT const VersionMap& ServerContextAllowedPendingCommand() noexcept;
OPENTXS_EXPORT const std::map<std::uint32_t, std::set<int>>&
ServerContextAllowedState() noexcept;
OPENTXS_EXPORT const std::map<std::uint32_t, std::set<int>>&
ServerContextAllowedStatus() noexcept;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYCONSENSUS_HPP
