// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/client/Blockchain.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <set>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"

// #define OT_METHOD
// "opentxs::api::client::implementation::Blockchain::BalanceOracle::"

namespace opentxs::api::client::implementation
{
Blockchain::BalanceOracle::BalanceOracle(
    const api::client::internal::Blockchain& parent,
    const api::Core& api) noexcept
    : parent_(parent)
    , api_(api)
    , zmq_(api_.ZeroMQ())
    , cb_(zmq::ListenCallback::Factory([this](auto& in) { cb(in); }))
    , socket_(zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind))
    , lock_()
    , subscribers_()
    , nym_subscribers_()
{
    const auto started = socket_->Start(api_.Endpoints().BlockchainBalance());

    OT_ASSERT(started);
}

auto Blockchain::BalanceOracle::cb(
    opentxs::network::zeromq::Message& in) noexcept -> void
{
    const auto& header = in.Header();

    OT_ASSERT(0 < header.size());

    const auto& connectionID = header.at(0);
    const auto body = in.Body();

    OT_ASSERT(1 < body.size());

    const auto haveNym = (2 < body.size());
    auto output = opentxs::blockchain::Balance{};
    const auto& chainFrame = body.at(1);
    const auto nym = [&] {
        auto output = api_.Factory().NymID();

        if (haveNym) {
            const auto& frame = body.at(2);
            output->Assign(frame.Bytes());
        }

        return output;
    }();
    auto postcondition = ScopeGuard{[&]() {
        auto message = zmq_.TaggedReply(in, WorkType::BlockchainBalance);
        message->AddFrame(chainFrame);
        message->AddFrame(output.first);
        message->AddFrame(output.second);

        if (haveNym) { message->AddFrame(nym); }

        socket_->Send(message);
    }};
    const auto chain = chainFrame.as<Chain>();
    const auto unsupported =
        (0 == opentxs::blockchain::SupportedChains().count(chain)) &&
        (Chain::UnitTest != chain);

    if (unsupported) { return; }

    try {
        const auto& network = parent_.GetChain(chain);

        if (haveNym) {
            output = network.GetBalance(nym);
        } else {
            output = network.GetBalance();
        }

        Lock lock(lock_);

        if (haveNym) {
            nym_subscribers_[chain][nym].emplace(
                api_.Factory().Data(connectionID.Bytes()));
        } else {
            subscribers_[chain].emplace(
                api_.Factory().Data(connectionID.Bytes()));
        }
    } catch (...) {
    }
}

auto Blockchain::BalanceOracle::RefreshBalance(
    const identifier::Nym& owner,
    const Chain chain) const noexcept -> void
{
    try {
        const auto& network = parent_.GetChain(chain);
        UpdateBalance(chain, network.GetBalance());
        UpdateBalance(owner, chain, network.GetBalance(owner));
    } catch (...) {
    }
}

auto Blockchain::BalanceOracle::UpdateBalance(
    const Chain chain,
    const Balance balance) const noexcept -> void
{
    auto cb = [&](const auto& in) {
        auto out = zmq_.Message(in);
        out->AddFrame();
        out->AddFrame(value(WorkType::BlockchainBalance));
        out->AddFrame(chain);
        out->AddFrame(balance.first);
        out->AddFrame(balance.second);
        socket_->Send(out);
    };
    Lock lock(lock_);
    const auto& subscribers = subscribers_[chain];
    std::for_each(std::begin(subscribers), std::end(subscribers), cb);
}

auto Blockchain::BalanceOracle::UpdateBalance(
    const identifier::Nym& owner,
    const Chain chain,
    const Balance balance) const noexcept -> void
{
    auto cb = [&](const auto& in) {
        auto out = zmq_.Message(in);
        out->AddFrame();
        out->AddFrame(value(WorkType::BlockchainBalance));
        out->AddFrame(chain);
        out->AddFrame(balance.first);
        out->AddFrame(balance.second);
        out->AddFrame(owner);
        socket_->Send(out);
    };
    Lock lock(lock_);
    const auto& subscribers = nym_subscribers_[chain][owner];
    std::for_each(std::begin(subscribers), std::end(subscribers), cb);
}
}  // namespace opentxs::api::client::implementation
