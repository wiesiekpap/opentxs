// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "ui/blockchainselection/BlockchainSelection.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <future>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/ui/Blockchains.hpp"
#if OT_QT
#include "opentxs/ui/qt/BlockchainSelection.hpp"
#endif  // OT_QT
#include "ui/base/List.hpp"

#define OT_METHOD "opentxs::ui::implementation::BlockchainSelection::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto BlockchainSelectionModel(
    const api::client::internal::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    -> std::unique_ptr<ui::implementation::BlockchainSelection>
{
    using ReturnType = ui::implementation::BlockchainSelection;

    return std::make_unique<ReturnType>(api, blockchain, type, cb);
}

#if OT_QT
auto BlockchainSelectionQtModel(
    ui::implementation::BlockchainSelection& parent) noexcept
    -> std::unique_ptr<ui::BlockchainSelectionQt>
{
    using ReturnType = ui::BlockchainSelectionQt;

    return std::make_unique<ReturnType>(parent);
}
#endif  // OT_QT
}  // namespace opentxs::factory

#if OT_QT
namespace opentxs::ui
{
QT_PROXY_MODEL_WRAPPER(
    BlockchainSelectionQt,
    implementation::BlockchainSelection)

auto BlockchainSelectionQt::disableChain(const int chain) const noexcept -> bool
{
    return parent_.Disable(static_cast<blockchain::Type>(chain));
}

auto BlockchainSelectionQt::enableChain(const int chain) const noexcept -> bool
{
    return parent_.Enable(static_cast<blockchain::Type>(chain));
}
}  // namespace opentxs::ui
#endif

namespace opentxs::ui::implementation
{
BlockchainSelection::BlockchainSelection(
    const api::client::internal::Manager& api,
    const api::client::internal::Blockchain& blockchain,
    const ui::Blockchains type,
    const SimpleCallback& cb) noexcept
    : BlockchainSelectionList(
          api,
          Identifier::Factory(),
          cb,
          false
#if OT_QT
          ,
          Roles{
              {BlockchainSelectionQt::TypeRole, "type"},
          },
          3
#endif
          )
    , Worker(api, {})
    , blockchain_(blockchain)
    , filter_(filter(type))
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
    const auto output = blockchain_.Disable(type);

    if (output) { process_state(type, false); }

    return output;
}

auto BlockchainSelection::Enable(const blockchain::Type type) const noexcept
    -> bool
{
    const auto output = blockchain_.Enable(type);

    if (output) { process_state(type, true); }

    return output;
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
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid message").Flush();

        OT_FAIL;
    }

    const auto work = body.at(0).as<Work>();

    switch (work) {
        case Work::statechange: {
            process_state(in);
        } break;
        case Work::init: {
            startup();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        case Work::shutdown: {
            running_->Off();
            shutdown(shutdown_promise_);
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unhandled type: ")(
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

    auto custom = CustomData{};
    custom.emplace_back(new bool{enabled});
    const_cast<BlockchainSelection&>(*this).add_item(
        chain,
        {blockchain::IsTestnet(chain), blockchain::DisplayString(chain)},
        custom);
}

auto BlockchainSelection::startup() noexcept -> void
{
    for (const auto& chain : filter_) {
        auto custom = CustomData{};
        custom.emplace_back(new bool{blockchain_.IsEnabled(chain)});
        add_item(
            chain,
            {blockchain::IsTestnet(chain), blockchain::DisplayString(chain)},
            custom);
    }

    finish_startup();
}

BlockchainSelection::~BlockchainSelection()
{
    wait_for_startup();
    stop_worker().get();
}
}  // namespace opentxs::ui::implementation
