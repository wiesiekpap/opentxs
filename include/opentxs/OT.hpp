// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OT_HPP
#define OPENTXS_OT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <map>
#include <string>

#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
class Context;
}  // namespace api

class OTCaller;
}  // namespace opentxs

namespace opentxs
{
/** Context accessor
 *
 *  Returns a reference to the context
 *
 *  \throws std::runtime_error if the context is not initialized
 */
OPENTXS_EXPORT const api::Context& Context();

/** Shut down context
 *
 *  Call this when the application is closing, after all OT operations
 *  are complete.
 */
OPENTXS_EXPORT void Cleanup();

/** Start up context
 *
 *  Returns a reference to the context singleton after it has been
 *  initialized.
 *
 *  Call this during application startup, before attempting any OT operation
 *
 *  \throws std::runtime_error if the context is already initialized
 */
OPENTXS_EXPORT const api::Context& InitContext(
    const ArgList& args = {},
    OTCaller* externalPasswordCallback = nullptr);

/** Wait on context shutdown
 *
 *  Blocks until the context has been shut down
 */
OPENTXS_EXPORT void Join();

using LicenseMap = std::map<std::string, std::string>;

OPENTXS_EXPORT const LicenseMap& LicenseData() noexcept;
}  // namespace opentxs
#endif
