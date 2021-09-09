// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                              // IWYU pragma: associated
#include "1_Internal.hpp"                            // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Base.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <functional>
#include <stdexcept>
#include <utility>

#include "network/blockchain/sync/Base.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"
#include "opentxs/network/blockchain/sync/Data.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/Query.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/protobuf/BlockchainP2PHello.pb.h"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "util/Container.hpp"

#define OT_METHOD "opentxs::network::blockchain::sync::Base::"

namespace opentxs::network::blockchain::sync
{
using LocalType = Base::Imp::LocalType;
using RemoteType = Base::Imp::RemoteType;
using ForwardMap = boost::container::flat_map<LocalType, RemoteType>;
using ReverseMap = boost::container::flat_map<RemoteType, LocalType>;

auto map() noexcept -> const ForwardMap&;
auto map() noexcept -> const ForwardMap&
{
    static const auto data = ForwardMap{
        {MessageType::sync_request, WorkType::SyncRequest},
        {MessageType::sync_ack, WorkType::SyncAcknowledgement},
        {MessageType::sync_reply, WorkType::SyncReply},
        {MessageType::new_block_header, WorkType::NewBlock},
        {MessageType::query, WorkType::SyncQuery},
    };

    return data;
}

auto reverse_map() noexcept -> const ReverseMap&;
auto reverse_map() noexcept -> const ReverseMap&
{
    static const auto data =
        reverse_arbitrary_map<LocalType, RemoteType, ReverseMap>(map());

    return data;
}
}  // namespace opentxs::network::blockchain::sync

namespace opentxs::network::blockchain::sync
{
Base::Imp::Imp(
    VersionNumber version,
    MessageType type,
    std::vector<State> state,
    std::string endpoint,
    std::vector<Block> blocks) noexcept
    : version_(version)
    , type_(type)
    , state_(std::move(state))
    , endpoint_(std::move(endpoint))
    , blocks_(std::move(blocks))
{
}

Base::Imp::Imp(MessageType type) noexcept
    : Imp(default_version_, type, {}, {}, {})
{
}

Base::Imp::Imp() noexcept
    : Imp(MessageType::error)
{
}

Base::Base(std::unique_ptr<Imp> imp) noexcept
    : imp_(std::move(imp))
{
    OT_ASSERT(imp_);
}

Base::Base() noexcept
    : Base(std::make_unique<Imp>())
{
}

auto Base::Imp::asAcknowledgement() const noexcept -> const Acknowledgement&
{
    static const auto blank = Acknowledgement{};

    return blank;
}

auto Base::Imp::asData() const noexcept -> const Data&
{
    static const auto blank = Data{};

    return blank;
}

auto Base::Imp::asQuery() const noexcept -> const Query&
{
    static const auto blank = Query{};

    return blank;
}

auto Base::Imp::asRequest() const noexcept -> const Request&
{
    static const auto blank = Request{};

    return blank;
}

auto Base::Imp::serialize(zeromq::Message& out) const noexcept -> bool
{
    if (false == serialize_type(out)) { return false; }

    try {
        const auto hello = [&] {
            auto out = proto::BlockchainP2PHello{};
            out.set_version(hello_version_);

            for (const auto& state : state_) {
                if (false == state.Serialize(*out.add_state())) {
                    throw std::runtime_error{""};
                }
            }

            return out;
        }();
        out.AddFrame(hello);
        out.AddFrame(endpoint_.data(), endpoint_.size());

        for (const auto& block : blocks_) {
            const auto data = [&] {
                auto out = proto::BlockchainP2PSync{};

                if (false == block.Serialize(out)) {
                    throw std::runtime_error{""};
                }

                return out;
            }();
            out.AddFrame(data);
        }
    } catch (...) {

        return false;
    }

    return true;
}

auto Base::Imp::serialize_type(zeromq::Message& out) const noexcept -> bool
{
    if (MessageType::error == type_) {
        LogOutput(OT_METHOD)(__func__)(": Invalid type").Flush();

        return false;
    }

    if (0u == out.size()) {
        out.AddFrame();
    } else if (0u != out.at(out.size() - 1u).size()) {
        // NOTE supplied message should either be empty or else have header
        // frames followed by an empty delimiter frame.
        LogOutput(OT_METHOD)(__func__)(": Invalid message").Flush();

        return false;
    }

    out.AddFrame(translate(type_));

    return true;
}

auto Base::Imp::translate(LocalType in) noexcept -> RemoteType
{
    return map().at(in);
}

auto Base::Imp::translate(RemoteType in) noexcept -> LocalType
{
    return reverse_map().at(in);
}

auto Base::asAcknowledgement() const noexcept -> const Acknowledgement&
{
    return imp_->asAcknowledgement();
}

auto Base::asData() const noexcept -> const Data& { return imp_->asData(); }

auto Base::asQuery() const noexcept -> const Query& { return imp_->asQuery(); }

auto Base::asRequest() const noexcept -> const Request&
{
    return imp_->asRequest();
}

auto Base::Serialize(zeromq::Message& out) const noexcept -> bool
{
    return imp_->serialize(out);
}

auto Base::Type() const noexcept -> MessageType { return imp_->type_; }

auto Base::Version() const noexcept -> VersionNumber { return imp_->version_; }

Base::~Base() = default;
}  // namespace opentxs::network::blockchain::sync
