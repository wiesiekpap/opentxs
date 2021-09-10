// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/Pimpl.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class PairEventCallback;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class PairEvent;
}  // namespace proto

class PairEventCallbackSwig;

using OTZMQPairEventCallback = Pimpl<network::zeromq::PairEventCallback>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class OPENTXS_EXPORT PairEventCallback : virtual public ListenCallback
{
public:
    using ReceiveCallback = std::function<void(const proto::PairEvent&)>;

    static auto Factory(ReceiveCallback callback) -> OTZMQPairEventCallback;
    static auto Factory(PairEventCallbackSwig* callback)
        -> opentxs::Pimpl<opentxs::network::zeromq::PairEventCallback>;

    ~PairEventCallback() override = default;

protected:
    PairEventCallback() = default;

private:
    friend OTZMQPairEventCallback;

#ifndef _WIN32
    auto clone() const -> PairEventCallback* override = 0;
#endif

    PairEventCallback(const PairEventCallback&) = delete;
    PairEventCallback(PairEventCallback&&) = delete;
    auto operator=(const PairEventCallback&) -> PairEventCallback& = delete;
    auto operator=(PairEventCallback&&) -> PairEventCallback& = delete;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
