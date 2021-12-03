// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "network/zeromq/ListenCallback.hpp"  // IWYU pragma: associated

#include <functional>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::ListenCallback>;

namespace opentxs::network::zeromq
{
auto ListenCallback::Factory(zeromq::ListenCallback::ReceiveCallback callback)
    -> OTZMQListenCallback
{
    return OTZMQListenCallback(new implementation::ListenCallback(callback));
}

auto ListenCallback::Factory() -> OTZMQListenCallback
{
    return OTZMQListenCallback(
        new implementation::ListenCallback([](zeromq::Message&&) -> void {}));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
ListenCallback::ListenCallback(zeromq::ListenCallback::ReceiveCallback callback)
    : execute_lock_()
    , callback_lock_()
    , callback_(callback)
{
    OT_ASSERT(callback_);
}

auto ListenCallback::clone() const -> ListenCallback*
{
    return new ListenCallback(callback_);
}

auto ListenCallback::Deactivate() const noexcept -> void
{
    static const auto null = [](zeromq::Message&&) {};
    auto rlock = rLock{execute_lock_};
    auto lock = Lock{callback_lock_};
    callback_ = null;
}

auto ListenCallback::Process(zeromq::Message&& message) const noexcept -> void
{
    auto rlock = rLock{execute_lock_};
    auto cb = [this] {
        auto lock = Lock{callback_lock_};

        return callback_;
    }();
    cb(std::move(message));
}

ListenCallback::~ListenCallback() {}
}  // namespace opentxs::network::zeromq::implementation
