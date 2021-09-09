// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "api/client/ui/UpdateManager.hpp"  // IWYU pragma: associated

#include <functional>
#include <map>
#include <mutex>
#include <utility>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/util/WorkType.hpp"

#define OT_METHOD "opentxs::api::client::ui::UpdateManager::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::ui
{
struct UpdateManager::Imp {
    auto ActivateUICallback(const Identifier& widget) const noexcept -> void
    {
        pipeline_->Push(widget.str());
    }
    auto ClearUICallbacks(const Identifier& widget) const noexcept -> void
    {
        auto lock = Lock{lock_};
        map_.erase(widget);
    }
    auto RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
        const noexcept -> void
    {
        if (cb) {
            auto lock = Lock{lock_};
            map_[widget].emplace_back(cb);
        }
    }

    Imp(const api::client::Manager& api) noexcept
        : api_(api)
        , lock_()
        , map_()
        , publisher_(api.Network().ZeroMQ().PublishSocket())
        , pipeline_(api.Network().ZeroMQ().Pipeline(api, [this](auto& in) {
            pipeline(in);
        }))
    {
        publisher_->Start(api_.Endpoints().WidgetUpdate());
    }

private:
    const api::client::Manager& api_;
    mutable std::mutex lock_;
    mutable std::map<OTIdentifier, std::vector<SimpleCallback>> map_;
    OTZMQPublishSocket publisher_;
    OTZMQPipeline pipeline_;

    auto pipeline(zmq::Message& in) noexcept -> void
    {
        if (0 == in.size()) { return; }

        const auto& frame = in.at(0);
        const auto id = api_.Factory().Identifier(frame);
        LogTrace(OT_METHOD)(__func__)(": Widget ")(id->str())(" updated.")
            .Flush();
        auto lock = Lock{lock_};
        auto it = map_.find(id);

        if (map_.end() == it) { return; }

        const auto& callbacks = it->second;

        for (const auto& cb : callbacks) {
            if (cb) { cb(); }
        }

        const auto& socket = publisher_.get();
        auto work = socket.Context().TaggedMessage(WorkType::UIModelUpdated);
        work->AddFrame(id);
        socket.Send(work);
    }
};

UpdateManager::UpdateManager(const api::client::Manager& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
    // WARNING: do not access api_.Wallet() during construction
}

auto UpdateManager::ActivateUICallback(const Identifier& widget) const noexcept
    -> void
{
    imp_->ActivateUICallback(widget);
}

auto UpdateManager::ClearUICallbacks(const Identifier& widget) const noexcept
    -> void
{
    imp_->ClearUICallbacks(widget);
}

auto UpdateManager::RegisterUICallback(
    const Identifier& widget,
    const SimpleCallback& cb) const noexcept -> void
{
    imp_->RegisterUICallback(widget, cb);
}

UpdateManager::~UpdateManager() = default;
}  // namespace opentxs::api::client::ui
