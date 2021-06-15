// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/network/zeromq/ListenCallback.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class ListenCallbackSwig;
}  // namespace opentxs

namespace opentxs::network::zeromq::implementation
{
class ListenCallbackSwig final : virtual public zeromq::ListenCallback
{
public:
    void Process(zeromq::Message& message) const final;

    ~ListenCallbackSwig() final;

private:
    friend zeromq::ListenCallback;

    opentxs::ListenCallbackSwig* callback_;

    auto clone() const -> ListenCallbackSwig* final;

    ListenCallbackSwig(opentxs::ListenCallbackSwig* callback);
    ListenCallbackSwig() = delete;
    ListenCallbackSwig(const ListenCallbackSwig&) = delete;
    ListenCallbackSwig(ListenCallbackSwig&&) = delete;
    auto operator=(const ListenCallbackSwig&) -> ListenCallbackSwig& = delete;
    auto operator=(ListenCallbackSwig&&) -> ListenCallbackSwig& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
