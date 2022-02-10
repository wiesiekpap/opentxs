// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>

#include "opentxs/network/zeromq/ListenCallback.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class ListenCallback final : virtual public zeromq::ListenCallback
{
public:
    auto Deactivate() const noexcept -> void final;
    auto Process(zeromq::Message&& message) const noexcept -> void final;

    ~ListenCallback() final;

private:
    friend zeromq::ListenCallback;

    mutable std::recursive_mutex execute_lock_;
    mutable std::mutex callback_lock_;
    mutable zeromq::ListenCallback::ReceiveCallback callback_;

    auto clone() const -> ListenCallback* final;

    ListenCallback(zeromq::ListenCallback::ReceiveCallback callback);
    ListenCallback() = delete;
    ListenCallback(const ListenCallback&) = delete;
    ListenCallback(ListenCallback&&) = delete;
    auto operator=(const ListenCallback&) -> ListenCallback& = delete;
    auto operator=(ListenCallback&&) -> ListenCallback& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
