// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "api/client/blockchain/BalanceOracle.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <map>
#include <mutex>
#include <set>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"

// #define OT_METHOD "opentxs::api::client::blockchain::BalanceOracle::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::api::client::blockchain
{
struct BalanceOracle::Imp {
    auto RefreshBalance(const identifier::Nym& owner, const Chain chain)
        const noexcept -> void
    {
        try {
            const auto& network = api_.Network().Blockchain().GetChain(chain);
            UpdateBalance(chain, network.GetBalance());
            UpdateBalance(owner, chain, network.GetBalance(owner));
        } catch (...) {
        }
    }

    auto UpdateBalance(const Chain chain, const Balance balance) const noexcept
        -> void
    {
        const auto make = [&](auto& out, auto type) {
            out->AddFrame();
            out->AddFrame(value(type));
            out->AddFrame(chain);
            out->AddFrame(balance.first);
            out->AddFrame(balance.second);
        };
        {
            auto out = zmq_.Message();
            make(out, WorkType::BlockchainWalletUpdated);
            publisher_->Send(out);
        }
        const auto notify = [&](const auto& in) {
            auto out = zmq_.Message(in);
            make(out, WorkType::BlockchainBalance);
            socket_->Send(out);
        };
        {
            auto lock = Lock{lock_};
            const auto& subscribers = subscribers_[chain];
            std::for_each(
                std::begin(subscribers), std::end(subscribers), notify);
        }
    }

    auto UpdateBalance(
        const identifier::Nym& owner,
        const Chain chain,
        const Balance balance) const noexcept -> void
    {
        const auto make = [&](auto& out, auto type) {
            out->AddFrame();
            out->AddFrame(value(type));
            out->AddFrame(chain);
            out->AddFrame(balance.first);
            out->AddFrame(balance.second);
            out->AddFrame(owner);
        };
        {
            auto out = zmq_.Message();
            make(out, WorkType::BlockchainWalletUpdated);
            publisher_->Send(out);
        }
        const auto notify = [&](const auto& in) {
            auto out = zmq_.Message(in);
            make(out, WorkType::BlockchainBalance);
            socket_->Send(out);
        };
        {
            auto lock = Lock{lock_};
            const auto& subscribers = nym_subscribers_[chain][owner];
            std::for_each(
                std::begin(subscribers), std::end(subscribers), notify);
        }
    }

    Imp(const api::Core& api) noexcept
        : api_(api)
        , zmq_(api_.Network().ZeroMQ())
        , cb_(zmq::ListenCallback::Factory([this](auto& in) { cb(in); }))
        , socket_([&] {
            auto out =
                zmq_.RouterSocket(cb_, zmq::socket::Socket::Direction::Bind);
            const auto started =
                out->Start(api_.Endpoints().BlockchainBalance());

            OT_ASSERT(started);

            return out;
        }())
        , publisher_([&] {
            auto out = zmq_.PublishSocket();
            const auto started =
                out->Start(api_.Endpoints().BlockchainWalletUpdated());

            OT_ASSERT(started);

            return out;
        }())
        , lock_()
        , subscribers_()
        , nym_subscribers_()
    {
    }

private:
    using Subscribers = std::set<OTData>;

    const api::Core& api_;
    const zmq::Context& zmq_;
    OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    OTZMQPublishSocket publisher_;
    mutable std::mutex lock_;
    mutable std::map<Chain, Subscribers> subscribers_;
    mutable std::map<Chain, std::map<OTNymID, Subscribers>> nym_subscribers_;

    auto cb(opentxs::network::zeromq::Message& in) noexcept -> void
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
            const auto& network = api_.Network().Blockchain().GetChain(chain);

            if (haveNym) {
                output = network.GetBalance(nym);
            } else {
                output = network.GetBalance();
            }

            auto lock = Lock{lock_};

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
};

BalanceOracle::BalanceOracle(const api::Core& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto BalanceOracle::RefreshBalance(
    const identifier::Nym& owner,
    const Chain chain) const noexcept -> void
{
    imp_->RefreshBalance(owner, chain);
}

auto BalanceOracle::UpdateBalance(const Chain chain, const Balance balance)
    const noexcept -> void
{
    imp_->UpdateBalance(chain, balance);
}

auto BalanceOracle::UpdateBalance(
    const identifier::Nym& owner,
    const Chain chain,
    const Balance balance) const noexcept -> void
{
    imp_->UpdateBalance(owner, chain, balance);
}

BalanceOracle::~BalanceOracle() = default;
}  // namespace opentxs::api::client::blockchain
