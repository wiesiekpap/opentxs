// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "network/zeromq/socket/Pipeline.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/network/zeromq/socket/Socket.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Pipeline>;

//"opentxs::network::zeromq::socket::implementation::Pipeline::"

namespace opentxs::factory
{
auto Pipeline(
    const api::Session& api,
    const network::zeromq::Context& context,
    std::function<void(network::zeromq::Message&)> callback)
    -> std::unique_ptr<opentxs::network::zeromq::Pipeline>
{
    return std::make_unique<
        opentxs::network::zeromq::socket::implementation::Pipeline>(
        api, context, callback);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Pipeline::Pipeline(
    const api::Session& api,
    const zeromq::Context& context,
    std::function<void(zeromq::Message&)> callback) noexcept
    : sender_(context.PushSocket(Socket::Direction::Bind))
    , callback_(ListenCallback::Factory(callback))
    , receiver_(context.SubscribeSocket(callback_))
{
    const auto endpoint = std::string("inproc://opentxs/") +
                          api.Crypto().Encode().Nonce(32)->Get();
    auto started = receiver_->Start(endpoint);
    started &= sender_->Start(endpoint);

    OT_ASSERT(started);
}

auto Pipeline::Close() const noexcept -> bool
{
    return sender_->Close() && receiver_->Close();
}

Pipeline::~Pipeline() { Close(); }
}  // namespace opentxs::network::zeromq::socket::implementation
