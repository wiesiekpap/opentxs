// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_NETWORK_ZAP_HPP
#define OPENTXS_API_NETWORK_ZAP_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/network/zeromq/zap/Callback.hpp"

namespace opentxs
{
namespace api
{
namespace network
{
class ZAP
{
public:
    using Callback = opentxs::network::zeromq::zap::Callback::ReceiveCallback;
    using Policy = opentxs::network::zeromq::zap::Callback::Policy;

    /** Set a callback that will be triggered for any ZAP requests in a
     *  specified domain
     *
     *   \param[in] domain  The ZAP domain to be registered. Any non-empty
     *                      string is valid.
     *   \param[in] callback    The callback to be executed for the specified
     *                          domain.
     *
     *   \return True if the domain is valid and not already registered
     */
    OPENTXS_EXPORT virtual bool RegisterDomain(
        const std::string& domain,
        const Callback& callback) const = 0;

    /** Configure ZAP policy for unhandled domains
     *
     *  Default behavior is Accept.
     *
     *  \param[in]  policy  Accept or reject ZAP requests for a domain which has
     *                      no registered callback
     */
    OPENTXS_EXPORT virtual bool SetDefaultPolicy(const Policy policy) const = 0;

    OPENTXS_EXPORT virtual ~ZAP() = default;

protected:
    ZAP() = default;

private:
    ZAP(const ZAP&) = delete;
    ZAP(ZAP&&) = delete;
    ZAP& operator=(const ZAP&) = delete;
    ZAP& operator=(ZAP&&) = delete;
};
}  // namespace network
}  // namespace api
}  // namespace opentxs
#endif
