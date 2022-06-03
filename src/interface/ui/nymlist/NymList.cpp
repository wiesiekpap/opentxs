// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "interface/ui/nymlist/NymList.hpp"  // IWYU pragma: associated

#include <atomic>
#include <future>
#include <memory>
#include <utility>

#include "interface/ui/base/List.hpp"
#include "internal/core/identifier/Identifier.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto NymListModel(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept -> std::unique_ptr<ui::internal::NymList>
{
    using ReturnType = ui::implementation::NymList;

    return std::make_unique<ReturnType>(api, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
NymList::NymList(
    const api::session::Client& api,
    const SimpleCallback& cb) noexcept
    : NymListList(api, api.Factory().Identifier(), cb, false)
    , Worker(api, {})
{
    init_executor({
        UnallocatedCString{api.Endpoints().NymCreated()},
        UnallocatedCString{api.Endpoints().NymDownload()},
    });
    pipeline_.Push(MakeWork(Work::init));
}

auto NymList::construct_row(
    const NymListRowID& id,
    const NymListSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::NymListItem(*this, Widget::api_, id, index, custom);
}

auto NymList::load() noexcept -> void
{
    for (auto& nym : Widget::api_.Wallet().LocalNyms()) {
        load(std::move(const_cast<OTNymID&&>(nym)));
    }
}

auto NymList::load(OTNymID&& id) noexcept -> void
{
    auto nym = Widget::api_.Wallet().Nym(id);
    auto custom = CustomData{};
    add_item(id, nym->Name(), custom);
}

auto NymList::pipeline(Message&& in) noexcept -> void
{
    if (false == running_.load()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    if ((false == startup_complete()) && (Work::init != work)) {
        pipeline_.Push(std::move(in));

        return;
    }

    switch (work) {
        case Work::shutdown: {
            protect_shutdown([this] { shut_down(); });
        } break;
        case Work::newnym: {
            process_new_nym(std::move(in));
        } break;
        case Work::nymchanged: {
            process_nym_changed(std::move(in));
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())("Unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto NymList::state_machine() noexcept -> bool { return false; }

auto NymList::shut_down() noexcept -> void
{
    close_pipeline();
    // TODO MT-34 investigate what other actions might be needed
}

auto NymList::process_new_nym(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    auto nymID = Widget::api_.Factory().NymID(body.at(1));

    OT_ASSERT(false == nymID->empty())

    load(std::move(nymID));
}

auto NymList::process_nym_changed(Message&& in) noexcept -> void
{
    const auto& api = Widget::api_;
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    auto nymID = api.Factory().NymID(body.at(1));

    OT_ASSERT(false == nymID->empty())

    if (false == api.Wallet().IsLocalNym(nymID)) { return; }

    load(std::move(nymID));
}

auto NymList::startup() noexcept -> void
{
    load();
    finish_startup();
    trigger();
}

NymList::~NymList()
{
    wait_for_startup();
    protect_shutdown([this] { shut_down(); });
}
}  // namespace opentxs::ui::implementation
