// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Index.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Index::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Actor(
          parent->api_,
          LogTrace(),
          0ms,
          batch,
          alloc,
          {
              {parent->to_children_endpoint_, Direction::Connect},
              {CString{
                   parent->api_.Crypto().Blockchain().Internal().KeyEndpoint(),
                   alloc},
               Direction::Connect},
          },
          {
              {parent->to_index_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->from_children_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_scan_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_rescan_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_process_endpoint_, Direction::Connect},
               }},
          })
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , to_scan_(pipeline_.Internal().ExtraSocket(1))
    , to_rescan_(pipeline_.Internal().ExtraSocket(2))
    , to_process_(pipeline_.Internal().ExtraSocket(3))
    , parent_p_(parent)
    , parent_(*parent_p_)
    , state_(State::normal)
    , last_indexed_(std::nullopt)
{
}

auto Index::Imp::done(
    const node::internal::WalletDatabase::ElementMap& elements) noexcept -> void
{
    const auto& db = parent_.db_;
    const auto& index = parent_.db_key_;
    db.SubchainAddElements(index, elements);
    last_indexed_ = parent_.db_.SubchainLastIndexed(index);
}

auto Index::Imp::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg));
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        default: {
            OT_FAIL;
        }
    }
}

auto Index::Imp::process_key(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(4u < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (chain != parent_.chain_) { return; }

    const auto owner = parent_.api_.Factory().NymID(body.at(2));

    if (owner != parent_.owner_) { return; }

    const auto id = parent_.api_.Factory().Identifier(body.at(3));

    if (id != parent_.id_) { return; }

    const auto subchain = body.at(4).as<crypto::Subchain>();

    if (subchain != parent_.subchain_) { return; }

    do_work();
}

auto Index::Imp::process_reorg(Message&& msg) noexcept -> void
{
    // NOTE no action required
}

auto Index::Imp::process_update(Message&& msg) noexcept -> void
{
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(parent_.api_, msg, clean, dirty);

    for (const auto& [type, position] : clean) {
        if (ScanState::processed == type) {
            log_(OT_PRETTY_CLASS())(parent_.name_)(" re-indexing ")(
                parent_.name_)(" due to processed block ")(
                opentxs::print(position))
                .Flush();
        }
    }

    do_work();
    to_rescan_.Send(std::move(msg));
}

auto Index::Imp::startup() noexcept -> void
{
    last_indexed_ = parent_.db_.SubchainLastIndexed(parent_.db_key_);
    do_work();
    log_(OT_PRETTY_CLASS())(parent_.name_)(" notifying scan task to begin work")
        .Flush();
    to_scan_.Send(MakeWork(Work::startup));
}

auto Index::Imp::state_normal(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::reorg_begin: {
            transition_state_reorg(std::move(msg));
        } break;
        case Work::reorg_end:
        case Work::do_reorg: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::update: {
            process_update(std::move(msg));
        } break;
        case Work::key: {
            process_key(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::startup:
        case Work::init:
        case Work::statemachine:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Index::Imp::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown:
        case Work::update:
        case Work::key:
        case Work::statemachine: {
            defer(std::move(msg));
        } break;
        case Work::reorg_end: {
            transition_state_normal(std::move(msg));
        } break;
        case Work::reorg_begin: {
            LogError()(OT_PRETTY_CLASS())(": wrong state for message ")(
                CString{print(work), get_allocator()})
                .Flush();

            OT_FAIL;
        }
        case Work::do_reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::reorg_begin_ack:
        case Work::reorg_end_ack:
        case Work::startup:
        case Work::init:
        default: {
            LogError()(OT_PRETTY_CLASS())(": unhandled type").Flush();

            OT_FAIL;
        }
    }
}

auto Index::Imp::transition_state_normal(Message&& msg) noexcept -> void
{
    disable_automatic_processing_ = false;
    state_ = State::normal;
    to_process_.Send(std::move(msg));
    flush_cache();
    do_work();
}

auto Index::Imp::transition_state_reorg(Message&& msg) noexcept -> void
{
    disable_automatic_processing_ = true;
    state_ = State::reorg;
    to_rescan_.Send(std::move(msg));
}

auto Index::Imp::work() noexcept -> bool
{
    const auto need = need_index(last_indexed_);

    if (need.has_value()) { process(last_indexed_, need.value()); }

    return false;
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Index::Index(boost::shared_ptr<Imp>&& imp) noexcept
    : imp_(std::move(imp))
{
    imp_->Init(imp_);
}

Index::Index(Index&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
}

Index::~Index() = default;
}  // namespace opentxs::blockchain::node::wallet
