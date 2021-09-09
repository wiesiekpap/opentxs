// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_REQUEST_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_REQUEST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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
class OPENTXS_EXPORT Request : virtual public zeromq::Message
{
public:
    static auto Factory() -> Pimpl<Request>;
    static auto Factory(
        const std::string& address,
        const std::string& domain,
        const zap::Mechanism mechanism,
        const Data& requestID = Data::Factory(),
        const Data& identity = Data::Factory(),
        const std::string& version = "1.0") -> Pimpl<Request>;

    virtual auto Address() const -> std::string = 0;
    virtual auto Credentials() const -> const FrameSection = 0;
    virtual auto Debug() const -> std::string = 0;
    virtual auto Domain() const -> std::string = 0;
    virtual auto Identity() const -> OTData = 0;
    virtual auto Mechanism() const -> zap::Mechanism = 0;
    virtual auto RequestID() const -> OTData = 0;
    virtual auto Validate() const -> std::pair<bool, std::string> = 0;
    virtual auto Version() const -> std::string = 0;

    virtual auto AddCredential(const Data& credential) -> Frame& = 0;

    ~Request() override = default;

protected:
    Request() = default;

private:
    friend OTZMQZAPRequest;

#ifndef _WIN32
    auto clone() const -> Request* override = 0;
#endif

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
