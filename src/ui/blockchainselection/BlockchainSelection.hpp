// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/util/WorkType.hpp"
#include "ui/base/List.hpp"
#include "ui/base/Widget.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

namespace network
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace network
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
    auto EnabledCount() const noexcept -> std::size_t final;
    auto Set(EnabledCallback&& cb) const noexcept -> void final;

    BlockchainSelection(
        const api::client::Manager& api,
        const api::network::internal::Blockchain& blockchain,
        const ui::Blockchains type,
        const SimpleCallback& cb) noexcept;

    ~BlockchainSelection() final;

private:
    friend Worker<BlockchainSelection>;

    struct Callback {
        auto run(blockchain::Type chain, bool enabled, std::size_t total)
            const noexcept -> void
        {
            auto lock = Lock{lock_};

            if (cb_) { cb_(chain, enabled, total); }
        }
        auto set(const EnabledCallback& cb) noexcept -> void
        {
            auto lock = Lock{lock_};
            cb_ = cb;
        }

    private:
        mutable std::mutex lock_{};
        EnabledCallback cb_{};
    };

    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        statechange = value(WorkType::BlockchainStateChange),
        enable = OT_ZMQ_INTERNAL_SIGNAL + 0,
        disable = OT_ZMQ_INTERNAL_SIGNAL + 1,
        init = OT_ZMQ_INIT_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const api::network::internal::Blockchain& blockchain_;
    const std::set<blockchain::Type> filter_;
    mutable std::map<blockchain::Type, bool> chain_state_;
    mutable std::atomic<std::size_t> enabled_count_;
    mutable Callback enabled_callback_;

    static auto filter(const ui::Blockchains type) noexcept
        -> std::set<blockchain::Type>;

    auto process_state(const blockchain::Type chain, const bool enabled)
        const noexcept -> void;

    auto construct_row(
        const BlockchainSelectionRowID& id,
        const BlockchainSelectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;
    auto disable(const Message& in) noexcept -> void;
    auto enable(const Message& in) noexcept -> void;
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
