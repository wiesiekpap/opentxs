// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "network/zeromq/PairEventCallback.hpp"  // IWYU pragma: associated

#include <functional>

#include "Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/PairEvent.pb.h"

template class opentxs::Pimpl<opentxs::network::zeromq::PairEventCallback>;

//"opentxs::network::zeromq::implementation::PairEventCallback::"

namespace opentxs::network::zeromq
{
auto PairEventCallback::Factory(
    zeromq::PairEventCallback::ReceiveCallback callback)
    -> OTZMQPairEventCallback
{
    return OTZMQPairEventCallback(
        new implementation::PairEventCallback(callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
PairEventCallback::PairEventCallback(
    zeromq::PairEventCallback::ReceiveCallback callback)
    : execute_lock_()
    , callback_lock_()
    , callback_(callback)
{
}

auto PairEventCallback::clone() const -> PairEventCallback*
{
    return new PairEventCallback(callback_);
}

auto PairEventCallback::Deactivate() noexcept -> void
{
    static const auto null = [](const proto::PairEvent&) {};
    auto rlock = rLock{execute_lock_};
    auto lock = Lock{callback_lock_};
    callback_ = null;
}

auto PairEventCallback::Process(zeromq::Message&& message) const noexcept
    -> void
{
    OT_ASSERT(1 == message.Body().size());

    const auto event = proto::Factory<proto::PairEvent>(message.Body_at(0));
    auto rlock = rLock{execute_lock_};
    auto cb = [this] {
        auto lock = Lock{callback_lock_};

        return callback_;
    }();
    cb(event);
}

PairEventCallback::~PairEventCallback() = default;
}  // namespace opentxs::network::zeromq::implementation
