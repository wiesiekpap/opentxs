// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_REQUEST_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_REQUEST_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <tuple>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/zap/ZAP.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network

using OTZMQZAPRequest = Pimpl<network::zeromq::zap::Request>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Request : virtual public zeromq::Message
{
public:
    OPENTXS_EXPORT static Pimpl<Request> Factory();
    OPENTXS_EXPORT static Pimpl<Request> Factory(
        const std::string& address,
        const std::string& domain,
        const zap::Mechanism mechanism,
        const Data& requestID = Data::Factory(),
        const Data& identity = Data::Factory(),
        const std::string& version = "1.0");

    OPENTXS_EXPORT virtual std::string Address() const = 0;
    OPENTXS_EXPORT virtual const FrameSection Credentials() const = 0;
    OPENTXS_EXPORT virtual std::string Debug() const = 0;
    OPENTXS_EXPORT virtual std::string Domain() const = 0;
    OPENTXS_EXPORT virtual OTData Identity() const = 0;
    OPENTXS_EXPORT virtual zap::Mechanism Mechanism() const = 0;
    OPENTXS_EXPORT virtual OTData RequestID() const = 0;
    OPENTXS_EXPORT virtual std::pair<bool, std::string> Validate() const = 0;
    OPENTXS_EXPORT virtual std::string Version() const = 0;

    OPENTXS_EXPORT virtual Frame& AddCredential(const Data& credential) = 0;

    OPENTXS_EXPORT ~Request() override = default;

protected:
    Request() = default;

private:
    friend OTZMQZAPRequest;

#ifndef _WIN32
    Request* clone() const override = 0;
#endif

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    Request& operator=(const Request&) = delete;
    Request& operator=(Request&&) = delete;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
