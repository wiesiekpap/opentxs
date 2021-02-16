// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <deque>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Network.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/protobuf/BlockchainP2PChainState.pb.h"
#include "opentxs/protobuf/BlockchainP2PHello.pb.h"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PChainState.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PHello.hpp"
#include "opentxs/protobuf/verify/BlockchainP2PSync.hpp"

namespace b = ot::blockchain;
namespace zmq = ot::network::zeromq;

namespace
{
using Dir = zmq::socket::Socket::Direction;
using Position = ot::blockchain::block::Position;
using Hello = ot::proto::BlockchainP2PHello;
using State = ot::proto::BlockchainP2PChainState;
using Sync = ot::proto::BlockchainP2PSync;

constexpr auto test_chain_{b::Type::UnitTest};
constexpr auto sync_server_sync_{"inproc://sync_server_endpoint/sync"};
constexpr auto sync_server_sync_public_{
    "inproc://sync_server_public_endpoint/sync"};
constexpr auto sync_server_update_{"inproc://sync_server_endpoint/update"};
constexpr auto sync_server_update_public_{
    "inproc://sync_server_public_endpoint/update"};

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
    using Height = b::block::Height;
    using Position = b::block::Position;

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

            if (height == target_) {
                try {
                    promise_.set_value(std::move(position));
                } catch (...) {
                }
            }
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

class MinedBlocks
{
public:
    using BlockHash = b::block::pHash;
    using Promise = std::promise<BlockHash>;
    using Future = std::shared_future<BlockHash>;

    [[maybe_unused]] auto get(const std::size_t index) const -> Future
    {
        auto lock = ot::Lock{lock_};

        if (index >= hashes_.size()) {
            throw std::out_of_range("Invalid index");
        }

        return hashes_.at(index);
    }

    [[maybe_unused]] auto allocate() noexcept -> Promise
    {
        auto lock = ot::Lock{lock_};
        auto promise = Promise{};
        hashes_.emplace_back(promise.get_future());

        return promise;
    }

private:
    using Vector = std::vector<Future>;

    mutable std::mutex lock_{};
    Vector hashes_{};
};

struct SyncRequestor {
    std::atomic_int checked_;
    std::atomic_int expected_;

    auto check(const State& state, const Position& pos) const noexcept -> bool
    {
        auto output = ot::proto::Validate(state, ot::VERBOSE);
        output &= (state.chain() == static_cast<std::uint32_t>(test_chain_));
        output &= (state.height() == static_cast<std::uint64_t>(pos.first));
        output &= (state.hash() == pos.second->Bytes());

        EXPECT_EQ(state.chain(), static_cast<std::uint32_t>(test_chain_));
        EXPECT_EQ(state.height(), static_cast<std::uint64_t>(pos.first));
        EXPECT_EQ(state.hash(), pos.second->Bytes());

        return output;
    }
    [[maybe_unused]] auto check(const State& state, const std::size_t index)
        const -> bool
    {
        auto pos = [&] {
            const auto& chain = api_.Blockchain().GetChain(test_chain_);
            auto header =
                chain.HeaderOracle().LoadHeader(cache_.get(index).get());

            OT_ASSERT(header);

            return header->Position();
        }();

        return check(state, pos);
    }
    [[maybe_unused]] auto check(
        const zmq::Frame& frame,
        const std::size_t index) const noexcept -> bool
    {
        constexpr auto filterType{
            ot::blockchain::filter::Type::Extended_opentxs};
        const auto& chain = api_.Blockchain().GetChain(test_chain_);
        const auto header =
            chain.HeaderOracle().LoadHeader(cache_.get(index).get());

        OT_ASSERT(header);

        auto serialize = std::string{};
        header->Serialize(ot::writer(serialize));
        const auto& pos = header->Position();
        const auto data = ot::proto::Factory<Sync>(frame);
        auto output = ot::proto::Validate(data, ot::VERBOSE);
        output &= (data.chain() == static_cast<std::uint32_t>(test_chain_));
        output &= (data.height() == static_cast<std::uint64_t>(pos.first));
        output &=
            (data.filter_type() == static_cast<std::uint32_t>(filterType));
        // TODO verify filter

        EXPECT_EQ(data.chain(), static_cast<std::uint32_t>(test_chain_));
        EXPECT_EQ(data.height(), static_cast<std::uint64_t>(pos.first));
        EXPECT_EQ(data.filter_type(), static_cast<std::uint32_t>(filterType));

        return output;
    }

    [[maybe_unused]] auto get(const std::size_t index) const
        -> const zmq::Message&
    {
        auto lock = ot::Lock{lock_};

        return buffer_.at(index);
    }
    [[maybe_unused]] auto request(const Position& pos) const noexcept -> bool
    {
        auto msg = api_.ZeroMQ().Message();
        msg->StartBody();
        msg->AddFrame([&] {
            auto hello = Hello{};
            hello.set_version(1);
            auto& state = *hello.add_state();
            state.set_version(1);
            state.set_chain(static_cast<std::uint32_t>(test_chain_));
            state.set_height(static_cast<std::uint64_t>(pos.first));
            const auto bytes = pos.second->Bytes();
            state.set_hash(bytes.data(), bytes.size());

            return hello;
        }());

        return socket_->Send(msg);
    }
    [[maybe_unused]] auto wait(const bool hard = true) noexcept -> bool
    {
        const auto limit =
            hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
        auto start = ot::Clock::now();

        while ((updated_ < expected_) && ((ot::Clock::now() - start) < limit)) {
            ot::Sleep(std::chrono::milliseconds(100));
        }

        if (false == hard) { updated_.store(expected_.load()); }

        return updated_ >= expected_;
    }

    SyncRequestor(
        const ot::api::client::Manager& api,
        const MinedBlocks& cache) noexcept
        : checked_(-1)
        , expected_(0)
        , api_(api)
        , cache_(cache)
        , lock_()
        , updated_(0)
        , buffer_()
        , cb_(zmq::ListenCallback::Factory([&](auto& msg) { cb(msg); }))
        , socket_(api.ZeroMQ().DealerSocket(cb_, Dir::Connect))
    {
        socket_->Start(sync_server_sync_);
    }

private:
    using Buffer = std::deque<ot::OTZMQMessage>;

    const ot::api::client::Manager& api_;
    const MinedBlocks& cache_;
    mutable std::mutex lock_;
    std::atomic_int updated_;
    Buffer buffer_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQDealerSocket socket_;

    auto cb(zmq::Message& in) noexcept -> void
    {
        auto lock = ot::Lock{lock_};
        buffer_.emplace_back(std::move(in));
        ++updated_;
    }
};

struct SyncSubscriber {
    using BlockHash = ot::blockchain::block::pHash;
    using SyncPromise = std::promise<BlockHash>;
    using SyncFuture = std::shared_future<BlockHash>;

    std::atomic_int expected_;

    [[maybe_unused]] auto wait(const bool hard = true) noexcept -> bool
    {
        if (wait_for_counter(hard)) {
            auto output = (0 == errors_);
            errors_ = 0;

            return output;
        }

        return false;
    }

    [[maybe_unused]] SyncSubscriber(
        const ot::api::client::Manager& api,
        const MinedBlocks& cache)
        : expected_(0)
        , api_(api)
        , cache_(cache)
        , updated_(0)
        , errors_(0)
        , cb_(ot::network::zeromq::ListenCallback::Factory(
              [&](const auto& in) { check_update(in); }))
        , socket_(api_.ZeroMQ().SubscribeSocket(cb_))
    {
        if (false == socket_->Start(sync_server_update_)) {
            throw std::runtime_error("Failed to subscribe to updates");
        }
    }

private:
    const ot::api::client::Manager& api_;
    const MinedBlocks& cache_;
    std::atomic_int updated_;
    std::atomic_int errors_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    auto wait_for_counter(const bool hard = true) noexcept -> bool
    {
        const auto limit =
            hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
        auto start = ot::Clock::now();

        while ((updated_ < expected_) && ((ot::Clock::now() - start) < limit)) {
            ot::Sleep(std::chrono::milliseconds(100));
        }

        if (false == hard) { updated_.store(expected_.load()); }

        return updated_ >= expected_;
    }

    auto check_update(const ot::network::zeromq::Message& in) noexcept -> void
    {
        auto body = in.Body();

        if (2 > body.size()) {
            ++errors_;

            return;
        }

        try {
            auto counter{-1};

            for (const auto& frame : body) {
                if (0 == ++counter) {
                    const auto hello = ot::proto::Factory<Hello>(frame);

                    if (false == ot::proto::Validate(hello, ot::VERBOSE)) {
                        throw std::runtime_error("invalid hello");
                    }

                    if (1 != hello.state().size()) {
                        throw std::runtime_error("wrong state count");
                    }

                    const auto& state = hello.state().at(0);
                    constexpr auto chain =
                        static_cast<std::uint32_t>(test_chain_);

                    if (state.chain() != chain) {
                        throw std::runtime_error("wrong chain");
                    }
                } else {
                    const auto state = ot::proto::Factory<State>(frame);

                    if (false == ot::proto::Validate(state, ot::VERBOSE)) {
                        throw std::runtime_error("invalid state");
                    }

                    const auto index = updated_++;
                    const auto future = cache_.get(index);
                    const auto hash = future.get();

                    if (state.hash() != hash->str()) {
                        std::runtime_error("wrong hash");
                    }
                }
            }
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            ++errors_;
        }
    }
};

class WalletListener
{
public:
    using Height = b::block::Height;

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

            if (height == target_) {
                try {
                    promise_.set_value(height);
                } catch (...) {
                }
            }
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

class Regtest_fixture_base : public ::testing::Test
{
protected:
    using Height = b::block::Height;
    using Transaction = ot::api::Factory::Transaction_p;
    using Generator = std::function<Transaction(Height)>;

    const ot::api::client::Manager& miner_;
    const ot::api::client::Manager& client_;
    const b::p2p::Address& address_;
    const PeerListener& connection_;
    const Generator default_;
    MinedBlocks& mined_blocks_;
    BlockListener& block_;
    WalletListener& wallet_;

    [[maybe_unused]] virtual auto Connect() noexcept -> bool
    {
        const auto& miner = miner_.Blockchain().GetChain(test_chain_);
        const auto listenMiner = miner.Listen(address_);

        EXPECT_TRUE(listenMiner);

        const auto& client = client_.Blockchain().GetChain(test_chain_);
        const auto listenClient = client.AddPeer(address_);

        EXPECT_TRUE(listenClient);

        const auto status = connection_.done_.wait_for(std::chrono::minutes{5});
        const auto future = (std::future_status::ready == status);

        EXPECT_TRUE(future);
        EXPECT_EQ(connection_.miner_peers_, 1);
        EXPECT_EQ(connection_.client_peers_, 1);

        return listenMiner && listenClient && future &&
               (1 == connection_.miner_peers_) &&
               (1 == connection_.client_peers_);
    }
    [[maybe_unused]] auto Mine(
        const Height ancestor,
        const std::size_t count) noexcept -> bool
    {
        return Mine(ancestor, count, default_);
    }
    [[maybe_unused]] auto Mine(
        const Height ancestor,
        const std::size_t count,
        const Generator& gen,
        const std::vector<Transaction>& extra = {}) noexcept -> bool
    {
        const auto targetHeight = ancestor + static_cast<Height>(count);
        auto blockFuture = block_.GetFuture(targetHeight);
        auto walletFuture = wallet_.GetFuture(targetHeight);
        const auto& network = miner_.Blockchain().GetChain(test_chain_);
        const auto& headerOracle = network.HeaderOracle();
        auto previousHeader =
            headerOracle.LoadHeader(headerOracle.BestHash(ancestor))
                ->as_Bitcoin();

        OT_ASSERT(previousHeader);

        for (auto i = std::size_t{0u}; i < count; ++i) {
            auto promise = mined_blocks_.allocate();

            OT_ASSERT(gen);

            auto tx = gen(previousHeader->Height() + 1);

            OT_ASSERT(tx);

            auto block = miner_.Factory().BitcoinBlock(
                *previousHeader,
                tx,
                previousHeader->nBits(),
                extra,
                previousHeader->Version(),
                [start{ot::Clock::now()}] {
                    return (ot::Clock::now() - start) > std::chrono::minutes(2);
                });

            OT_ASSERT(block);

            promise.set_value(block->Header().Hash());
            const auto added = network.AddBlock(block);

            OT_ASSERT(added);

            previousHeader = block->Header().as_Bitcoin();

            OT_ASSERT(previousHeader);
        }

        auto output = true;
        constexpr auto limit = std::chrono::minutes(5);
        using Status = std::future_status;

        {
            OT_ASSERT(blockFuture.wait_for(limit) == Status::ready);

            const auto [height, hash] = blockFuture.get();

            EXPECT_EQ(hash, previousHeader->Hash());

            output &= (hash == previousHeader->Hash());
        }

        {
            OT_ASSERT(walletFuture.wait_for(limit) == Status::ready);

            const auto height = walletFuture.get();

            EXPECT_EQ(height, targetHeight);

            output &= (height == targetHeight);
        }

        return output;
    }

    [[maybe_unused]] virtual auto Shutdown() noexcept -> void
    {
        wallet_listener_.reset();
        block_listener_.reset();
        mined_block_cache_.reset();
        peer_listener_.reset();
        listen_address_.reset();
    }
    [[maybe_unused]] auto Start() noexcept -> bool
    {
        const auto startMiner = miner_.Blockchain().Start(test_chain_);
        const auto startClient = client_.Blockchain().Start(test_chain_);

        EXPECT_TRUE(startMiner);
        EXPECT_TRUE(startClient);

        return startMiner && startClient;
    }

    [[maybe_unused]] Regtest_fixture_base(const ot::ArgList& clientArgs)
        : miner_(ot::Context().StartClient(
              [] {
                  auto args = OTTestEnvironment::test_args_;
                  auto& level = args[OPENTXS_ARG_BLOCK_STORAGE_LEVEL];
                  level.clear();
                  level.emplace("2");
                  auto& wallet = args["disableblockchainwallet"];
                  wallet.emplace("true");

                  return args;
              }(),
              0))
        , client_(ot::Context().StartClient(clientArgs, 1))
        , address_(init_address(miner_))
        , connection_(init_peer(miner_, client_))
        , default_([&](Height height) -> Transaction {
            using OutputBuilder = ot::api::Factory::OutputBuilder;

            return miner_.Factory().BitcoinGenerationTransaction(
                test_chain_, height, [&] {
                    auto output = std::vector<OutputBuilder>{};
                    const auto text = std::string{"null"};
                    const auto keys =
                        std::set<ot::api::client::blockchain::Key>{};
                    output.emplace_back(
                        5000000000,
                        miner_.Factory().BitcoinScriptNullData(
                            test_chain_, {text}),
                        keys);

                    return output;
                }());
        })
        , mined_blocks_(init_mined())
        , block_(init_block(client_))
        , wallet_(init_wallet(client_))
    {
    }

private:
    static std::unique_ptr<const ot::OTBlockchainAddress> listen_address_;
    static std::unique_ptr<const PeerListener> peer_listener_;
    static std::unique_ptr<MinedBlocks> mined_block_cache_;
    static std::unique_ptr<BlockListener> block_listener_;
    static std::unique_ptr<WalletListener> wallet_listener_;

    static auto init_address(const ot::api::Core& api) noexcept
        -> const b::p2p::Address&
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
                    test_chain_,
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
    static auto init_mined() noexcept -> MinedBlocks&
    {
        if (false == bool(mined_block_cache_)) {
            mined_block_cache_ = std::make_unique<MinedBlocks>();
        }

        OT_ASSERT(mined_block_cache_);

        return *mined_block_cache_;
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

class Regtest_fixture_normal : public Regtest_fixture_base
{
protected:
    [[maybe_unused]] Regtest_fixture_normal()
        : Regtest_fixture_base([] {
            auto args = OTTestEnvironment::test_args_;
            auto& level = args[OPENTXS_ARG_BLOCK_STORAGE_LEVEL];
            level.clear();
            level.emplace("0");

            return args;
        }())
    {
    }
};

class Regtest_fixture_sync : public Regtest_fixture_base
{
protected:
    SyncSubscriber& sync_sub_;
    SyncRequestor& sync_req_;

    [[maybe_unused]] auto Connect() noexcept -> bool final
    {
        auto output = Regtest_fixture_base::Connect();
        output &= client_.Blockchain().StartSyncServer(
            sync_server_sync_,
            sync_server_sync_public_,
            sync_server_update_,
            sync_server_update_public_);

        return output;
    }

    [[maybe_unused]] auto Shutdown() noexcept -> void final
    {
        sync_requestor_.reset();
        sync_subscriber_.reset();
        Regtest_fixture_base::Shutdown();
    }

    [[maybe_unused]] Regtest_fixture_sync()
        : Regtest_fixture_base([] {
            auto args = OTTestEnvironment::test_args_;
            auto& level = args[OPENTXS_ARG_BLOCK_STORAGE_LEVEL];
            level.clear();
            level.emplace("2");
            auto& sync = args[OPENTXS_ARG_BLOCKCHAIN_SYNC];
            sync.clear();
            sync.emplace();

            return args;
        }())
        , sync_sub_(
              [&]() -> SyncSubscriber& {
                  if (!sync_subscriber_) {
                      sync_subscriber_ = std::make_unique<SyncSubscriber>(
                          client_, mined_blocks_);
                  }

                  return *sync_subscriber_;
              }()

                  )
        , sync_req_(
              [&]() -> SyncRequestor& {
                  if (!sync_requestor_) {
                      sync_requestor_ = std::make_unique<SyncRequestor>(
                          client_, mined_blocks_);
                  }

                  return *sync_requestor_;
              }()

          )
    {
    }

private:
    static std::unique_ptr<SyncSubscriber> sync_subscriber_;
    static std::unique_ptr<SyncRequestor> sync_requestor_;
};
}  // namespace

namespace
{
std::unique_ptr<const ot::OTBlockchainAddress>
    Regtest_fixture_base::listen_address_{};
std::unique_ptr<const PeerListener> Regtest_fixture_base::peer_listener_{};
std::unique_ptr<MinedBlocks> Regtest_fixture_base::mined_block_cache_{};
std::unique_ptr<BlockListener> Regtest_fixture_base::block_listener_{};
std::unique_ptr<WalletListener> Regtest_fixture_base::wallet_listener_{};
std::unique_ptr<SyncSubscriber> Regtest_fixture_sync::sync_subscriber_{};
std::unique_ptr<SyncRequestor> Regtest_fixture_sync::sync_requestor_{};
}  // namespace
