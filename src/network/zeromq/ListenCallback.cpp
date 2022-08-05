// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "network/zeromq/ListenCallback.hpp"  // IWYU pragma: associated

#include <functional>
#include <memory>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
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
    , callback_(callback ? callback : null_)
{
    auto handle = callback_.lock();
    const auto& cb = *handle;

    OT_ASSERT(cb);
}

auto ListenCallback::clone() const -> ListenCallback*
{
    return new ListenCallback(*callback_.lock());
}

auto ListenCallback::Deactivate() noexcept -> void
{
    //    auto rlock = rLock{execute_lock_};
    //    *callback_.lock() = nullptr;
    Replace(nullptr);
    //    const_cast<ListenCallback&>(*this).Replace(null_);
}

auto ListenCallback::Process(zeromq::Message&& message) const noexcept -> void
{
    auto rlock = rLock{execute_lock_};
    // TODO this copy may be relatively costly.
    // It is here to prevent execution of callback with the lock on,
    // because Worker occasionally returns here asking recursively for the lock
    // in order to remove the callback.
    // Remove the copy when everything has been moved to Actor.
    auto cb = *callback_.lock();
    cb(std::move(message));
}

auto ListenCallback::Replace(ReceiveCallback cb) noexcept -> void
{
    auto rlock = rLock{execute_lock_};
    auto handle = callback_.lock();
    auto& callback = *handle;

    if (cb) {
        callback = std::move(cb);
    } else {
        callback = null_;
    }
}

ListenCallback::~ListenCallback() {}
}  // namespace opentxs::network::zeromq::implementation
