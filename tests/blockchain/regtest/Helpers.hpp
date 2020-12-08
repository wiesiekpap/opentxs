// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/Forward.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Network.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace b = ot::blockchain;
namespace zmq = ot::network::zeromq;

namespace
{
class PeerListener
{
    std::promise<void> promise_;

public:
    std::future<void> done_;
    std::atomic_int miner_peers_;
    std::atomic_int client_peers_;

    PeerListener(
        const ot::api::client::Manager& miner,
        const ot::api::client::Manager& client)
        : promise_()
        , done_(promise_.get_future())
        , miner_peers_(0)
        , client_peers_(0)
        , miner_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&) { cb(miner_peers_); }))
        , client_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&) { cb(client_peers_); }))
        , m_socket_(miner.ZeroMQ().SubscribeSocket(miner_cb_))
        , c_socket_(client.ZeroMQ().SubscribeSocket(client_cb_))
    {
        if (false == m_socket_->Start(miner.Endpoints().BlockchainPeer())) {
            throw std::runtime_error("Error connecting to miner socket");
        }

        if (false == c_socket_->Start(client.Endpoints().BlockchainPeer())) {
            throw std::runtime_error("Error connecting to client socket");
        }
    }

    ~PeerListener()
    {
        c_socket_->Close();
        m_socket_->Close();
    }

private:
    ot::OTZMQListenCallback miner_cb_;
    ot::OTZMQListenCallback client_cb_;
    ot::OTZMQSubscribeSocket m_socket_;
    ot::OTZMQSubscribeSocket c_socket_;

    auto cb(std::atomic_int& counter) noexcept -> void
    {
        ++counter;

        if ((0 < miner_peers_) && (0 < client_peers_)) { promise_.set_value(); }
    }
};

class BlockListener
{
public:
    using Height = ot::blockchain::block::Height;
    using Position = ot::blockchain::block::Position;

    [[maybe_unused]] auto GetFuture(const Height height) noexcept
    {
        auto lock = ot::Lock{lock_};
        target_ = height;

        try {
            promise_ = {};
        } catch (...) {
        }

        return promise_.get_future();
    }

    BlockListener(const ot::api::Core& api) noexcept
        : api_(api)
        , lock_()
        , promise_()
        , target_(-1)
        , cb_(zmq::ListenCallback::Factory([&](const zmq::Message& msg) {
            const auto body = msg.Body();

            OT_ASSERT(body.size() == 4);

            auto lock = ot::Lock{lock_};
            auto position = Position{body.at(3).as<Height>(), [&] {
                                         auto output = api_.Factory().Data();
                                         output->Assign(body.at(2).Bytes());

                                         return output;
                                     }()};
            const auto& [height, hash] = position;

            if (height == target_) { promise_.set_value(std::move(position)); }
        }))
        , socket_(api_.ZeroMQ().SubscribeSocket(cb_))
    {
        OT_ASSERT(socket_->Start(api_.Endpoints().BlockchainReorg()));
    }

private:
    const ot::api::Core& api_;
    mutable std::mutex lock_;
    std::promise<Position> promise_;
    Height target_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;
};

class WalletListener
{
public:
    using Height = ot::blockchain::block::Height;

    [[maybe_unused]] auto GetFuture(const Height height) noexcept
    {
        auto lock = ot::Lock{lock_};
        target_ = height;

        try {
            promise_ = {};
        } catch (...) {
        }

        return promise_.get_future();
    }

    WalletListener(const ot::api::Core& api) noexcept
        : api_(api)
        , lock_()
        , promise_()
        , target_(-1)
        , cb_(zmq::ListenCallback::Factory([&](const zmq::Message& msg) {
            const auto body = msg.Body();

            OT_ASSERT(body.size() == 4);

            auto lock = ot::Lock{lock_};
            const auto height = body.at(2).as<Height>();

            if (height == target_) { promise_.set_value(height); }
        }))
        , socket_(api_.ZeroMQ().SubscribeSocket(cb_))
    {
        OT_ASSERT(socket_->Start(api_.Endpoints().BlockchainSyncProgress()));
    }

private:
    const ot::api::Core& api_;
    mutable std::mutex lock_;
    std::promise<Height> promise_;
    Height target_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;
};

class Regtest_fixture : public ::testing::Test
{
protected:
    static constexpr auto chain_{ot::blockchain::Type::UnitTest};

    const ot::api::client::Manager& miner_;
    const ot::api::client::Manager& client_;
    const ot::blockchain::p2p::Address& address_;
    const PeerListener& connection_;
    BlockListener& block_;
    WalletListener& wallet_;

    [[maybe_unused]] auto Connect() noexcept -> bool
    {
        const auto& miner = miner_.Blockchain().GetChain(chain_);
        const auto listenMiner = miner.Listen(address_);

        EXPECT_TRUE(listenMiner);

        const auto& client = client_.Blockchain().GetChain(b::Type::UnitTest);
        const auto listenClient = client.AddPeer(address_);

        EXPECT_TRUE(listenClient);

        const auto status =
            connection_.done_.wait_for(std::chrono::seconds{30});
        const auto future = (std::future_status::ready == status);

        EXPECT_TRUE(future);
        EXPECT_EQ(connection_.miner_peers_, 1);
        EXPECT_EQ(connection_.client_peers_, 1);

        return listenMiner && listenClient && future &&
               (1 == connection_.miner_peers_) &&
               (1 == connection_.client_peers_);
    }
    auto Shutdown() noexcept
    {
        wallet_listener_.reset();
        block_listener_.reset();
        peer_listener_.reset();
    }
    [[maybe_unused]] auto Start() noexcept -> bool
    {
        const auto startMiner = miner_.Blockchain().Start(chain_);
        const auto startclient = client_.Blockchain().Start(chain_);

        EXPECT_TRUE(startMiner);
        EXPECT_TRUE(startclient);

        return startMiner && startclient;
    }

    Regtest_fixture()
        : miner_(ot::Context().StartClient(
              [] {
                  auto args = OTTestEnvironment::test_args_;
                  auto& level = args[OPENTXS_ARG_BLOCK_STORAGE_LEVEL];
                  level.clear();
                  level.emplace("2");

                  return args;
              }(),
              0))
        , client_(ot::Context().StartClient(
              [] {
                  auto args = OTTestEnvironment::test_args_;
                  auto& level = args[OPENTXS_ARG_BLOCK_STORAGE_LEVEL];
                  level.clear();
                  level.emplace("0");

                  return args;
              }(),
              1))
        , address_(init_address(miner_))
        , connection_(init_peer(miner_, client_))
        , block_(init_block(client_))
        , wallet_(init_wallet(client_))
    {
    }

private:
    static std::unique_ptr<const ot::OTBlockchainAddress> listen_address_;
    static std::unique_ptr<const PeerListener> peer_listener_;
    static std::unique_ptr<BlockListener> block_listener_;
    static std::unique_ptr<WalletListener> wallet_listener_;

    static auto init_address(const ot::api::Core& api) noexcept
        -> const ot::blockchain::p2p::Address&
    {
        constexpr auto test_endpoint{"inproc://test_endpoint"};
        constexpr auto test_port = std::uint16_t{18444};

        if (false == bool(listen_address_)) {
            listen_address_ = std::make_unique<ot::OTBlockchainAddress>(
                api.Factory().BlockchainAddress(
                    b::p2p::Protocol::bitcoin,
                    b::p2p::Network::zmq,
                    api.Factory().Data(
                        std::string{test_endpoint}, ot::StringStyle::Raw),
                    test_port,
                    b::Type::UnitTest,
                    {},
                    {}));
        }

        OT_ASSERT(listen_address_);

        return *listen_address_;
    }
    static auto init_block(const ot::api::Core& api) noexcept -> BlockListener&
    {
        if (false == bool(block_listener_)) {
            block_listener_ = std::make_unique<BlockListener>(api);
        }

        OT_ASSERT(block_listener_);

        return *block_listener_;
    }
    static auto init_peer(
        const ot::api::client::Manager& miner,
        const ot::api::client::Manager& client) noexcept -> const PeerListener&
    {
        if (false == bool(peer_listener_)) {
            peer_listener_ = std::make_unique<PeerListener>(miner, client);
        }

        OT_ASSERT(peer_listener_);

        return *peer_listener_;
    }
    static auto init_wallet(const ot::api::Core& api) noexcept
        -> WalletListener&
    {
        if (false == bool(wallet_listener_)) {
            wallet_listener_ = std::make_unique<WalletListener>(api);
        }

        OT_ASSERT(wallet_listener_);

        return *wallet_listener_;
    }
};
}  // namespace

namespace
{
std::unique_ptr<const ot::OTBlockchainAddress>
    Regtest_fixture::listen_address_{};
std::unique_ptr<const PeerListener> Regtest_fixture::peer_listener_{};
std::unique_ptr<BlockListener> Regtest_fixture::block_listener_{};
std::unique_ptr<WalletListener> Regtest_fixture::wallet_listener_{};
}  // namespace
