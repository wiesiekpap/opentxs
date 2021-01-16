// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/ui/BlockchainSelection.cpp"

#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "util/Work.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/util/WorkType.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class Factory;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using BlockchainSelectionList = List<
    BlockchainSelectionExternalInterface,
    BlockchainSelectionInternalInterface,
    BlockchainSelectionRowID,
    BlockchainSelectionRowInterface,
    BlockchainSelectionRowInternal,
    BlockchainSelectionRowBlank,
    BlockchainSelectionSortKey,
    BlockchainSelectionPrimaryID>;

class BlockchainSelection final : public BlockchainSelectionList,
                                  Worker<BlockchainSelection>
{
public:
    auto Disable(const blockchain::Type type) const noexcept -> bool final;
    auto Enable(const blockchain::Type type) const noexcept -> bool final;

    BlockchainSelection(
        const api::client::internal::Manager& api,
        const api::client::internal::Blockchain& blockchain,
        const ui::Blockchains type,
        const SimpleCallback& cb) noexcept;

    ~BlockchainSelection() final;

private:
    friend Worker<BlockchainSelection>;

    enum class Work : OTZMQWorkType {
        statechange = value(WorkType::BlockchainStateChange),
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        shutdown = value(WorkType::Shutdown),
    };

    const api::client::internal::Blockchain& blockchain_;
    const std::set<blockchain::Type> filter_;

    static auto filter(const ui::Blockchains type) noexcept
        -> std::set<blockchain::Type>;

    auto process_state(const blockchain::Type chain, const bool enabled)
        const noexcept -> void;

    auto construct_row(
        const BlockchainSelectionRowID& id,
        const BlockchainSelectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto pipeline(const Message& in) noexcept -> void;
    auto process_state(const Message& in) noexcept -> void;
    auto startup() noexcept -> void;

    BlockchainSelection() = delete;
    BlockchainSelection(const BlockchainSelection&) = delete;
    BlockchainSelection(BlockchainSelection&&) = delete;
    auto operator=(const BlockchainSelection&) -> BlockchainSelection& = delete;
    auto operator=(BlockchainSelection&&) -> BlockchainSelection& = delete;
};
}  // namespace opentxs::ui::implementation
