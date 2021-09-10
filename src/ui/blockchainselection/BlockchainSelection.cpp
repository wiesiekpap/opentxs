// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainselection/BlockchainSelection.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "internal/api/network/Network.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "ui/base/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::BlockchainSelection::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto BlockchainSelectionModel(
    const api::client::Manager& api,
    const api::network::internal::Blockchain& blockchain,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::internal::BlockchainSelection>
{
    using ReturnType = ui::implementation::BlockchainSelection;

    return std::make_unique<ReturnType>(api, blockchain, type, cb);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
BlockchainSelection::BlockchainSelection(
    const api::client::Manager& api,
    const api::network::internal::Blockchain& blockchain,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    : BlockchainSelectionList(api, Identifier::Factory(), cb, false)
    , Worker(api, {})
    , blockchain_(blockchain)
    , filter_(filter(type))
    , chain_state_([&] {
        auto out = std::map<blockchain::Type, bool>{};

        for (const auto chain : filter_) { out[chain] = false; }

        return out;
    }())
    , enabled_count_(0)
    , enabled_callback_()
{
    init_executor({api.Endpoints().BlockchainStateChange()});
    pipeline_->Push(MakeWork(Work::init));
}

auto BlockchainSelection::construct_row(
    const BlockchainSelectionRowID& id,
    const BlockchainSelectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::BlockchainSelectionItem(
        *this, Widget::api_, id, index, custom);
}

auto BlockchainSelection::Disable(const blockchain::Type type) const noexcept
    -> bool
{
    const auto work = [&] {
        auto out = Widget::api_.Network().ZeroMQ().TaggedMessage(Work::disable);
        out->AddFrame(type);

        return out;
    }();
    pipeline_->Push(work);

    return true;
}

auto BlockchainSelection::disable(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    process_state(chain, false);
    Widget::api_.Network().Blockchain().Disable(chain);
}

auto BlockchainSelection::Enable(const blockchain::Type type) const noexcept
    -> bool
{
    const auto work = [&] {
        auto out = Widget::api_.Network().ZeroMQ().TaggedMessage(Work::enable);
        out->AddFrame(type);

        return out;
    }();
    pipeline_->Push(work);

    return true;
}

auto BlockchainSelection::enable(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();
    process_state(chain, true);
    Widget::api_.Network().Blockchain().Enable(chain);
}

auto BlockchainSelection::EnabledCount() const noexcept -> std::size_t
{
    return enabled_count_.load();
}

auto BlockchainSelection::filter(const ui::Blockchains type) noexcept
    -> std::set<blockchain::Type>
{
    auto complete = blockchain::SupportedChains();

    switch (type) {
        case Blockchains::Main: {
            auto output = decltype(complete){};

            for (const auto& chain : complete) {
                if (false == blockchain::IsTestnet(chain)) {
                    output.emplace(chain);
                }
            }

            return output;
        }
        case Blockchains::Test: {
            auto output = decltype(complete){};

            for (const auto& chain : complete) {
                if (blockchain::IsTestnet(chain)) { output.emplace(chain); }
            }

            return output;
        }
        case Blockchains::All:
        default: {

            return complete;
        }
    }
}

auto BlockchainSelection::pipeline(const Message& in) noexcept -> void
{
    if (false == running_.get()) { return; }

    const auto body = in.Body();

    if (1 > body.size()) {
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = [&] {
        try {

            return body.at(0).as<Work>();
        } catch (...) {

            OT_FAIL;
        }
    }();

    switch (work) {
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::enable: {
            enable(in);
        } break;
        case Work::disable: {
            disable(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogOutput(OT_METHOD)(__func__)(": Unhandled type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto BlockchainSelection::process_state(const Message& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2 < body.size());

    process_state(body.at(1).as<blockchain::Type>(), body.at(2).as<bool>());
}

auto BlockchainSelection::process_state(
    const blockchain::Type chain,
    const bool enabled) const noexcept -> void
{
    if (0 == filter_.count(chain)) { return; }

    auto& isEnabled = chain_state_.at(chain);

    if (isEnabled) {
        if (enabled) {
            // Do nothing
        } else {
            isEnabled = false;
            --enabled_count_;
            enabled_callback_.run(chain, enabled, enabled_count_.load());
        }
    } else {
        if (enabled) {
            isEnabled = true;
            ++enabled_count_;
            enabled_callback_.run(chain, enabled, enabled_count_.load());
        } else {
            // Do nothing
        }
    }

    auto custom = CustomData{};
    custom.emplace_back(new bool{enabled});
    const_cast<BlockchainSelection&>(*this).add_item(
        chain,
        {blockchain::IsTestnet(chain), blockchain::DisplayString(chain)},
        custom);
}

auto BlockchainSelection::Set(EnabledCallback&& cb) const noexcept -> void
{
    enabled_callback_.set(std::move(cb));
}

auto BlockchainSelection::startup() noexcept -> void
{
    for (const auto& chain : filter_) {
        process_state(chain, blockchain_.IsEnabled(chain));
    }

    finish_startup();
}

BlockchainSelection::~BlockchainSelection()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
