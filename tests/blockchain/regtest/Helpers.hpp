// Copyright (c) 2010-2021 The Open-Transactions developers
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
#include "opentxs/api/client/blockchain/BalanceList.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Network.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/client/Wallet.hpp"
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
#include "opentxs/util/WorkType.hpp"

namespace b = ot::blockchain;
namespace zmq = ot::network::zeromq;

namespace
{
using Dir = zmq::socket::Socket::Direction;
using Position = ot::blockchain::block::Position;
using Hello = ot::proto::BlockchainP2PHello;
using State = ot::proto::BlockchainP2PChainState;
using Sync = ot::proto::BlockchainP2PSync;
using Pattern = ot::blockchain::block::bitcoin::Script::Pattern;
using FilterType = ot::blockchain::filter::Type;

constexpr auto test_chain_{b::Type::UnitTest};
constexpr auto sync_server_sync_{"inproc://sync_server_endpoint/sync"};
constexpr auto sync_server_sync_public_{
    "inproc://sync_server_public_endpoint/sync"};
constexpr auto sync_server_update_{"inproc://sync_server_endpoint/update"};
constexpr auto sync_server_update_public_{
    "inproc://sync_server_public_endpoint/update"};
constexpr auto coinbase_fun_{"The Industrial Revolution and its consequences "
                             "have been a disaster for the human race."};

class PeerListener
{
    std::promise<void> promise_;

public:
    std::future<void> done_;
    std::atomic_int miner_peers_;
    std::atomic_int client_1_peers_;
    std::atomic_int client_2_peers_;

    PeerListener(
        const int clientCount,
        const ot::api::client::Manager& miner,
        const ot::api::client::Manager& client1,
        const ot::api::client::Manager& client2)
        : promise_()
        , done_(promise_.get_future())
        , miner_peers_(0)
        , client_1_peers_(0)
        , client_2_peers_(0)
        , client_count_(clientCount)
        , lock_()
        , miner_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&) { cb(miner_peers_); }))
        , client_1_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&) { cb(client_1_peers_); }))
        , client_2_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto&) { cb(client_2_peers_); }))
        , m_socket_(miner.ZeroMQ().SubscribeSocket(miner_cb_))
        , c1_socket_(client1.ZeroMQ().SubscribeSocket(client_1_cb_))
        , c2_socket_(client2.ZeroMQ().SubscribeSocket(client_2_cb_))
    {
        if (false == m_socket_->Start(miner.Endpoints().BlockchainPeer())) {
            throw std::runtime_error("Error connecting to miner socket");
        }

        if (false == c1_socket_->Start(client1.Endpoints().BlockchainPeer())) {
            throw std::runtime_error("Error connecting to client1 socket");
        }

        if (false == c2_socket_->Start(client2.Endpoints().BlockchainPeer())) {
            throw std::runtime_error("Error connecting to client2 socket");
        }
    }

    ~PeerListener()
    {
        c2_socket_->Close();
        c1_socket_->Close();
        m_socket_->Close();
    }

private:
    const int client_count_;
    mutable std::mutex lock_;
    ot::OTZMQListenCallback miner_cb_;
    ot::OTZMQListenCallback client_1_cb_;
    ot::OTZMQListenCallback client_2_cb_;
    ot::OTZMQSubscribeSocket m_socket_;
    ot::OTZMQSubscribeSocket c1_socket_;
    ot::OTZMQSubscribeSocket c2_socket_;

    auto cb(std::atomic_int& counter) noexcept -> void
    {
        ++counter;

        auto lock = ot::Lock{lock_};

        if (client_count_ != miner_peers_) { return; }

        if ((0 < client_count_) && (0 == client_1_peers_)) { return; }

        if ((1 < client_count_) && (0 == client_2_peers_)) { return; }

        promise_.set_value();
    }
};

class BlockListener
{
public:
    using Height = b::block::Height;
    using Position = b::block::Position;
    using Future = std::future<Position>;

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
        constexpr auto filterType{ot::blockchain::filter::Type::ES};
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
        auto msg = api_.ZeroMQ().TaggedMessage(ot::WorkType::SyncRequest);
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

        if (3 > body.size()) {
            ++errors_;

            return;
        }

        try {
            auto counter{-1};

            for (const auto& frame : body) {
                switch (++counter) {
                    case 0: {
                        const auto type = frame.as<ot::WorkType>();

                        if (type != ot::WorkType::NewBlock) {
                            throw std::runtime_error("invalid type");
                        }
                    } break;
                    case 1: {
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
                    } break;
                    case 2: {
                        if (0 == frame.size()) {
                            throw std::runtime_error(
                                "invalid previous cfheader");
                        }
                    } break;
                    default: {
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
    using Future = std::future<Height>;

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

struct ScanListener {
    using Callback = ot::network::zeromq::ListenCallback;
    using Account = ot::api::client::blockchain::BalanceNode;
    using Subchain = ot::api::client::blockchain::Subchain;
    using Height = ot::blockchain::block::Height;
    using Future = std::future<void>;

    [[maybe_unused]] auto wait(const Future& future) const noexcept -> bool
    {
        constexpr auto limit = std::chrono::minutes{5};
        using Status = std::future_status;

        if (Status::ready == future.wait_for(limit)) {

            return true;
        } else {

            return false;
        }
    }

    [[maybe_unused]] auto get_future(
        const Account& account,
        Subchain subchain,
        Height target) noexcept -> Future
    {
        auto lock = ot::Lock{lock_};
        const auto& nym = account.Parent().NymID();
        const auto chain = account.Parent().Parent().Chain();
        const auto& id = account.ID();
        auto& map = map_[nym][chain][id];
        auto it = [&] {
            if (auto i = map.find(subchain); i != map.end()) {

                return i;
            } else {
                return map.try_emplace(subchain, -1, api_.Factory().Data())
                    .first;
            }
        }();

        return it->second.reset(target);
    }

    [[maybe_unused]] ScanListener(const ot::api::Core& api) noexcept
        : api_(api)
        , cb_(Callback::Factory([&](auto& msg) { cb(msg); }))
        , socket_([&] {
            auto out = api_.ZeroMQ().SubscribeSocket(cb_);
            const auto rc =
                out->Start(api_.Endpoints().BlockchainScanProgress());

            OT_ASSERT(rc);

            return out;
        }())
        , lock_()
        , map_()
    {
    }

private:
    using Chain = ot::blockchain::Type;
    using Position = ot::blockchain::block::Position;
    using Promise = std::promise<void>;

    struct Data {
        Position pos_;
        Height target_;
        Promise promise_;

        auto reset(Height target) noexcept -> Future
        {
            target_ = target;
            promise_ = {};

            return promise_.get_future();
        }
        auto test() noexcept -> void
        {
            if (pos_.first == target_) {
                try {
                    promise_.set_value();
                } catch (...) {
                }
            }
        }

        Data(Height height, ot::OTData&& hash) noexcept
            : pos_(height, std::move(hash))
            , target_()
            , promise_()
        {
        }
        [[maybe_unused]] Data(Data&&) = default;

    private:
        Data() = delete;
        Data(const Data&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;
    };

    using SubchainMap = std::map<Subchain, Data>;
    using AccountMap = std::map<ot::OTIdentifier, SubchainMap>;
    using ChainMap = std::map<Chain, AccountMap>;
    using Map = std::map<ot::OTNymID, ChainMap>;

    const ot::api::Core& api_;
    const ot::OTZMQListenCallback cb_;
    const ot::OTZMQSubscribeSocket socket_;
    mutable std::mutex lock_;
    Map map_;

    auto cb(const ot::network::zeromq::Message& in) noexcept -> void
    {
        const auto body = in.Body();

        OT_ASSERT(body.size() == 7u);

        const auto chain = body.at(1).as<Chain>();
        auto nymID = [&] {
            auto out = api_.Factory().NymID();
            out->Assign(body.at(2).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        auto accountID = [&] {
            auto out = api_.Factory().Identifier();
            out->Assign(body.at(3).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        const auto sub = body.at(4).as<Subchain>();
        const auto height = body.at(5).as<Height>();
        auto hash = [&] {
            auto out = api_.Factory().Data();
            out->Assign(body.at(6).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        auto lock = ot::Lock{lock_};
        auto& map = map_[std::move(nymID)][chain][std::move(accountID)];
        auto it = [&] {
            if (auto i = map.find(sub); i != map.end()) {
                if (height > i->second.pos_.first) {
                    i->second.pos_ = Position{height, std::move(hash)};
                }

                return i;
            } else {
                return map.try_emplace(sub, height, std::move(hash)).first;
            }
        }();

        it->second.test();
    }
};

class Regtest_fixture_base : public ::testing::Test
{
protected:
    using Height = b::block::Height;
    using Transaction = ot::api::Factory::Transaction_p;
    using Generator = std::function<Transaction(Height)>;
    using Outpoint = ot::blockchain::block::bitcoin::Outpoint;
    using Script = ot::blockchain::block::bitcoin::Script;
    using UTXO = ot::blockchain::client::Wallet::UTXO;
    using GetAmount = std::function<std::int64_t(int)>;
    using GetPattern = std::function<Pattern(int)>;
    using GetBytes =
        std::function<std::optional<ot::ReadView>(const Script&, int)>;

    const ot::ArgList client_args_;
    const int client_count_;
    const ot::api::client::Manager& miner_;
    const ot::api::client::Manager& client_1_;
    const ot::api::client::Manager& client_2_;
    const b::p2p::Address& address_;
    const PeerListener& connection_;
    const Generator default_;
    MinedBlocks& mined_blocks_;
    BlockListener& block_1_;
    BlockListener& block_2_;
    WalletListener& wallet_1_;
    WalletListener& wallet_2_;

    [[maybe_unused]] virtual auto Connect() noexcept -> bool
    {
        const auto miner = [&]() -> std::function<bool()> {
            const auto& miner = miner_.Blockchain().GetChain(test_chain_);
            const auto listen = miner.Listen(address_);

            EXPECT_TRUE(listen);

            return [=] {
                EXPECT_EQ(connection_.miner_peers_, client_count_);

                return listen && (client_count_ == connection_.miner_peers_);
            };
        }();
        const auto client1 = [&]() -> std::function<bool()> {
            if (0 < client_count_) {
                const auto& client =
                    client_1_.Blockchain().GetChain(test_chain_);
                const auto added = client.AddPeer(address_);

                EXPECT_TRUE(added);

                return [=] {
                    EXPECT_EQ(connection_.client_1_peers_, 1);

                    return added && (1 == connection_.client_1_peers_);
                };
            } else {

                return [] { return true; };
            }
        }();
        const auto client2 = [&]() -> std::function<bool()> {
            if (1 < client_count_) {
                const auto& client =
                    client_2_.Blockchain().GetChain(test_chain_);
                const auto added = client.AddPeer(address_);

                EXPECT_TRUE(added);

                return [=] {
                    EXPECT_EQ(connection_.client_2_peers_, 1);

                    return added && (1 == connection_.client_2_peers_);
                };
            } else {

                return [] { return true; };
            }
        }();

        static constexpr auto limit = std::chrono::seconds{30};
        const auto status = connection_.done_.wait_for(limit);
        const auto future = (std::future_status::ready == status);

        OT_ASSERT(future);

        return future && miner() && client1() && client2();
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

        auto blocks = std::vector<BlockListener::Future>{};
        auto wallets = std::vector<WalletListener::Future>{};
        blocks.reserve(client_count_);
        wallets.reserve(client_count_);

        if (0 < client_count_) {
            blocks.emplace_back(block_1_.GetFuture(targetHeight));
            wallets.emplace_back(wallet_1_.GetFuture(targetHeight));
        }

        if (1 < client_count_) {
            blocks.emplace_back(block_2_.GetFuture(targetHeight));
            wallets.emplace_back(wallet_2_.GetFuture(targetHeight));
        }

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

            EXPECT_TRUE(added);

            previousHeader = block->Header().as_Bitcoin();

            OT_ASSERT(previousHeader);
        }

        auto output = true;
        constexpr auto limit = std::chrono::minutes(30);
        using Status = std::future_status;

        for (auto& future : blocks) {
            OT_ASSERT(future.wait_for(limit) == Status::ready);

            const auto [height, hash] = future.get();

            EXPECT_EQ(hash, previousHeader->Hash());

            output &= (hash == previousHeader->Hash());
        }

        for (auto& future : wallets) {
            OT_ASSERT(future.wait_for(limit) == Status::ready);

            const auto height = future.get();

            EXPECT_EQ(height, targetHeight);

            output &= (height == targetHeight);
        }

        return output;
    }
    [[maybe_unused]] auto TestUTXOs(
        const std::vector<Outpoint>& outpoints,
        const std::vector<ot::OTData>& keys,
        const std::vector<UTXO>& utxos,
        const GetBytes getData,
        const GetPattern getPattern,
        const GetAmount amount) const noexcept -> bool
    {
        auto out = true;
        auto index{-1};

        for (const auto& [outpoint, pOutput] : utxos) {
            const auto& expected = outpoints.at(++index);
            out &= (outpoint == expected);
            EXPECT_EQ(outpoint.str(), expected.str());
            EXPECT_TRUE(pOutput);

            if (!pOutput) { return false; }

            const auto& output = *pOutput;
            out &= (output.Value() == amount(index));

            EXPECT_EQ(output.Value(), amount(index));

            const auto& script = output.Script();
            using Position = ot::blockchain::block::bitcoin::Script::Position;
            out &= (script.Role() == Position::Output);

            EXPECT_EQ(script.Role(), Position::Output);

            const auto data = getData(script, index);
            const auto pattern = getPattern(index);

            EXPECT_TRUE(data.has_value());

            if (false == data.has_value()) { return false; }

            out &= (data.value() == keys.at(index)->Bytes());
            out &= (script.Type() == pattern);

            EXPECT_EQ(data.value(), keys.at(index)->Bytes());
            EXPECT_EQ(script.Type(), pattern);
        }

        return out;
    }

    [[maybe_unused]] virtual auto Shutdown() noexcept -> void
    {
        wallet_listener_.clear();
        block_listener_.clear();
        mined_block_cache_.reset();
        peer_listener_.reset();
        listen_address_.reset();
    }
    [[maybe_unused]] auto Start() noexcept -> bool
    {
        const auto miner = [&]() -> std::function<bool()> {
            const auto start = miner_.Blockchain().Start(test_chain_);

            return [=] {
                EXPECT_TRUE(start);

                return start;
            };
        }();
        const auto client1 = [&]() -> std::function<bool()> {
            if (0 < client_count_) {
                const auto start = client_1_.Blockchain().Start(test_chain_);

                return [=] {
                    EXPECT_TRUE(start);

                    return start;
                };
            } else {

                return [] { return true; };
            }
        }();
        const auto client2 = [&]() -> std::function<bool()> {
            if (1 < client_count_) {
                const auto start = client_2_.Blockchain().Start(test_chain_);

                return [=] {
                    EXPECT_TRUE(start);

                    return start;
                };
            } else {

                return [] { return true; };
            }
        }();

        return miner() && client1() && client2();
    }

    [[maybe_unused]] Regtest_fixture_base(
        const int clientCount,
        const ot::ArgList& clientArgs)
        : client_args_(clientArgs)
        , client_count_(clientCount)
        , miner_(ot::Context().StartClient(
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
        , client_1_(ot::Context().StartClient(client_args_, 1))
        , client_2_(ot::Context().StartClient(client_args_, 2))
        , address_(init_address(miner_))
        , connection_(init_peer(client_count_, miner_, client_1_, client_2_))
        , default_([&](Height height) -> Transaction {
            using OutputBuilder = ot::api::Factory::OutputBuilder;

            return miner_.Factory().BitcoinGenerationTransaction(
                test_chain_,
                height,
                [&] {
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
                }(),
                coinbase_fun_);
        })
        , mined_blocks_(init_mined())
        , block_1_(init_block(0, client_1_))
        , block_2_(init_block(1, client_2_))
        , wallet_1_(init_wallet(0, client_1_))
        , wallet_2_(init_wallet(1, client_2_))
    {
    }

private:
    using BlockListen = std::map<int, std::unique_ptr<BlockListener>>;
    using WalletListen = std::map<int, std::unique_ptr<WalletListener>>;

    static std::unique_ptr<const ot::OTBlockchainAddress> listen_address_;
    static std::unique_ptr<const PeerListener> peer_listener_;
    static std::unique_ptr<MinedBlocks> mined_block_cache_;
    static BlockListen block_listener_;
    static WalletListen wallet_listener_;

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
    static auto init_block(const int index, const ot::api::Core& api) noexcept
        -> BlockListener&
    {
        auto& p = block_listener_[index];

        if (false == bool(p)) { p = std::make_unique<BlockListener>(api); }

        OT_ASSERT(p);

        return *p;
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
        const int clientCount,
        const ot::api::client::Manager& miner,
        const ot::api::client::Manager& client1,
        const ot::api::client::Manager& client2) noexcept -> const PeerListener&
    {
        if (false == bool(peer_listener_)) {
            peer_listener_ = std::make_unique<PeerListener>(
                clientCount, miner, client1, client2);
        }

        OT_ASSERT(peer_listener_);

        return *peer_listener_;
    }
    static auto init_wallet(const int index, const ot::api::Core& api) noexcept
        -> WalletListener&
    {
        auto& p = wallet_listener_[index];

        if (false == bool(p)) { p = std::make_unique<WalletListener>(api); }

        OT_ASSERT(p);

        return *p;
    }
};

class Regtest_fixture_normal : public Regtest_fixture_base
{
protected:
    [[maybe_unused]] Regtest_fixture_normal(const int clientCount)
        : Regtest_fixture_base(clientCount, [] {
            auto args = OTTestEnvironment::test_args_;
            auto& level = args[OPENTXS_ARG_BLOCK_STORAGE_LEVEL];
            level.clear();
            level.emplace("1");

            return args;
        }())
    {
    }
};

class Regtest_fixture_single : public Regtest_fixture_normal
{
protected:
    [[maybe_unused]] Regtest_fixture_single()
        : Regtest_fixture_normal(1)
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
        output &= client_1_.Blockchain().StartSyncServer(
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
        : Regtest_fixture_base(
              1,
              [] {
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
                          client_1_, mined_blocks_);
                  }

                  return *sync_subscriber_;
              }()

                  )
        , sync_req_(
              [&]() -> SyncRequestor& {
                  if (!sync_requestor_) {
                      sync_requestor_ = std::make_unique<SyncRequestor>(
                          client_1_, mined_blocks_);
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
Regtest_fixture_base::BlockListen Regtest_fixture_base::block_listener_{};
Regtest_fixture_base::WalletListen Regtest_fixture_base::wallet_listener_{};
std::unique_ptr<SyncSubscriber> Regtest_fixture_sync::sync_subscriber_{};
std::unique_ptr<SyncRequestor> Regtest_fixture_sync::sync_requestor_{};
}  // namespace
