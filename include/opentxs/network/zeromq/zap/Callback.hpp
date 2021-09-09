// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_CALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_CALLBACK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Callback;
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network

using OTZMQZAPCallback = Pimpl<network::zeromq::zap::Callback>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class OPENTXS_EXPORT Callback
{
public:
    using ReceiveCallback = std::function<OTZMQZAPReply(const Request&)>;

    enum class Policy : bool { Accept = true, Reject = false };

    static auto Factory(
        const std::string& domain,
        const ReceiveCallback& callback) -> OTZMQZAPCallback;
    static auto Factory() -> OTZMQZAPCallback;

    virtual auto Process(const Request& request) const -> OTZMQZAPReply = 0;
    virtual auto SetDomain(
        const std::string& domain,
        const ReceiveCallback& callback) const -> bool = 0;
    virtual auto SetPolicy(const Policy policy) const -> bool = 0;

    virtual ~Callback() = default;

protected:
    Callback() = default;

private:
    friend OTZMQZAPCallback;

    virtual auto clone() const -> Callback* = 0;

    Callback(const Callback&) = delete;
    Callback(Callback&&) = default;
    auto operator=(const Callback&) -> Callback& = delete;
    auto operator=(Callback&&) -> Callback& = default;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
