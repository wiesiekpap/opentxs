// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACK_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/Pimpl.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

class ListenCallbackSwig;

using OTZMQListenCallback = Pimpl<network::zeromq::ListenCallback>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class OPENTXS_EXPORT ListenCallback
{
public:
    using ReceiveCallback = std::function<void(Message&)>;

    static auto Factory(ReceiveCallback callback) -> OTZMQListenCallback;
    static auto Factory() -> OTZMQListenCallback;
    static auto Factory(ListenCallbackSwig* callback)
        -> opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>;

    virtual void Process(Message& message) const = 0;

    virtual ~ListenCallback() = default;

protected:
    ListenCallback() = default;

private:
    friend OTZMQListenCallback;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const -> ListenCallback* = 0;
#ifdef _WIN32
private:
#endif

    ListenCallback(const ListenCallback&) = delete;
    ListenCallback(ListenCallback&&) = default;
    auto operator=(const ListenCallback&) -> ListenCallback& = delete;
    auto operator=(ListenCallback&&) -> ListenCallback& = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
