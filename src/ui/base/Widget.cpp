// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "ui/base/Widget.hpp"  // IWYU pragma: associated

#include <iosfwd>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"

namespace opentxs::ui::implementation
{
auto verify_empty(const CustomData& custom) noexcept -> bool
{
    auto counter = std::ptrdiff_t{-1};

    for (const auto& ptr : custom) {
        ++counter;

        if (nullptr != ptr) {
            LogOutput("opentxs::ui::implementation::")(__func__)(
                ": unused pointer at index ")(counter)
                .Flush();

            return false;
        }
    }

    return true;
}

Widget::Widget(
    const api::client::Manager& api,
    const Identifier& id,
    const SimpleCallback& cb) noexcept
    : api_(api)
    , widget_id_(id)
    , ui_(api_.InternalClient().InternalUI())
    , callbacks_()
    , listeners_()
{
    if (cb) { SetCallback(cb); }
}

auto Widget::setup_listeners(const ListenerDefinitions& definitions) noexcept
    -> void
{
    for (const auto& [endpoint, functor] : definitions) {
        const auto* copy{functor};
        auto& nextCallback =
            callbacks_.emplace_back(network::zeromq::ListenCallback::Factory(
                [=](const Message& message) -> void {
                    (*copy)(this, message);
                }));
        auto& socket = listeners_.emplace_back(
            api_.Network().ZeroMQ().SubscribeSocket(nextCallback.get()));
        const auto listening = socket->Start(endpoint);

        OT_ASSERT(listening)
    }
}

Widget::~Widget() { ClearCallbacks(); }
}  // namespace opentxs::ui::implementation
