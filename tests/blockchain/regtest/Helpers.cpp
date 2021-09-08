// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

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
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/api/Options.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/crypto/Wallet.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/network/blockchain/sync/Data.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/Request.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace ottest
{
using Dir = zmq::socket::Socket::Direction;

constexpr auto sync_server_sync_{"inproc://sync_server_endpoint/sync"};
constexpr auto sync_server_sync_public_{
    "inproc://sync_server_public_endpoint/sync"};
constexpr auto sync_server_update_{"inproc://sync_server_endpoint/update"};

struct BlockListener::Imp {
    const ot::api::Core& api_;
    mutable std::mutex lock_;
    std::promise<Position> promise_;
    Height target_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    Imp(const ot::api::Core& api)
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
        , socket_(api_.Network().ZeroMQ().SubscribeSocket(cb_))
    {
        OT_ASSERT(socket_->Start(api_.Endpoints().BlockchainReorg()));
    }
};

BlockListener::BlockListener(const ot::api::Core& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto BlockListener::GetFuture(const Height height) noexcept
{
    auto lock = ot::Lock{imp_->lock_};
    imp_->target_ = height;

    try {
        imp_->promise_ = {};
    } catch (...) {
    }

    return imp_->promise_.get_future();
}

BlockListener::~BlockListener() = default;

struct MinedBlocks::Imp {
    using Vector = std::vector<Future>;

    mutable std::mutex lock_{};
    Vector hashes_{};
};

MinedBlocks::MinedBlocks()
    : imp_(std::make_unique<Imp>())
{
}

auto MinedBlocks::allocate() noexcept -> Promise
{
    auto lock = ot::Lock{imp_->lock_};
    auto promise = Promise{};
    imp_->hashes_.emplace_back(promise.get_future());

    return promise;
}

auto MinedBlocks::get(const std::size_t index) const -> Future
{
    auto lock = ot::Lock{imp_->lock_};

    if (index >= imp_->hashes_.size()) {
        throw std::out_of_range("Invalid index");
    }

    return imp_->hashes_.at(index);
}

MinedBlocks::~MinedBlocks() = default;

struct PeerListener::Imp {
    PeerListener& parent_;
    const int client_count_;
    mutable std::mutex lock_;
    ot::OTZMQListenCallback miner_cb_;
    ot::OTZMQListenCallback client_1_cb_;
    ot::OTZMQListenCallback client_2_cb_;
    ot::OTZMQSubscribeSocket m_socket_;
    ot::OTZMQSubscribeSocket c1_socket_;
    ot::OTZMQSubscribeSocket c2_socket_;

    auto cb(const zmq::Message& msg, std::atomic_int& counter) noexcept -> void
    {
        const auto body = msg.Body();

        OT_ASSERT(2 < body.size());

        if (0 == body.at(2).size()) { return; }

        ++counter;

        auto lock = ot::Lock{lock_};

        if (client_count_ != parent_.miner_peers_) { return; }

        if ((0 < client_count_) && (0 == parent_.client_1_peers_)) { return; }

        if ((1 < client_count_) && (0 == parent_.client_2_peers_)) { return; }

        try {
            parent_.promise_.set_value();
        } catch (...) {
        }
    }

    Imp(PeerListener& parent,
        const int clientCount,
        const ot::api::client::Manager& miner,
        const ot::api::client::Manager& client1,
        const ot::api::client::Manager& client2)
        : parent_(parent)
        , client_count_(clientCount)
        , lock_()
        , miner_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto& msg) { cb(msg, parent_.miner_peers_); }))
        , client_1_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto& msg) { cb(msg, parent_.client_1_peers_); }))
        , client_2_cb_(ot::network::zeromq::ListenCallback::Factory(
              [this](auto& msg) { cb(msg, parent_.client_2_peers_); }))
        , m_socket_(miner.Network().ZeroMQ().SubscribeSocket(miner_cb_))
        , c1_socket_(client1.Network().ZeroMQ().SubscribeSocket(client_1_cb_))
        , c2_socket_(client2.Network().ZeroMQ().SubscribeSocket(client_2_cb_))
    {
        if (false ==
            m_socket_->Start(miner.Endpoints().BlockchainPeerConnection())) {
            throw std::runtime_error("Error connecting to miner socket");
        }

        if (false ==
            c1_socket_->Start(client1.Endpoints().BlockchainPeerConnection())) {
            throw std::runtime_error("Error connecting to client1 socket");
        }

        if (false ==
            c2_socket_->Start(client2.Endpoints().BlockchainPeerConnection())) {
            throw std::runtime_error("Error connecting to client2 socket");
        }
    }

    ~Imp()
    {
        c2_socket_->Close();
        c1_socket_->Close();
        m_socket_->Close();
    }
};

PeerListener::PeerListener(
    const int clientCount,
    const ot::api::client::Manager& miner,
    const ot::api::client::Manager& client1,
    const ot::api::client::Manager& client2)
    : promise_()
    , done_(promise_.get_future())
    , miner_peers_(0)
    , client_1_peers_(0)
    , client_2_peers_(0)
    , imp_(std::make_unique<Imp>(*this, clientCount, miner, client1, client2))
{
}

PeerListener::~PeerListener() = default;

Regtest_fixture_base::Regtest_fixture_base(
    const int clientCount,
    const ot::Options& minerArgs,
    const ot::Options& clientArgs)
    : ot_(ot::Context())
    , client_args_(clientArgs)
    , client_count_(clientCount)
    , miner_(ot_.StartClient(
          ot::Options{minerArgs}
              .SetBlockchainStorageLevel(2)
              .SetBlockchainWalletEnabled(false),
          0))
    , client_1_(ot_.StartClient(client_args_, 1))
    , client_2_(ot_.StartClient(client_args_, 2))
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
                const auto keys = std::set<ot::blockchain::crypto::Key>{};
                output.emplace_back(
                    5000000000,
                    miner_.Factory().BitcoinScriptNullData(test_chain_, {text}),
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

Regtest_fixture_base::Regtest_fixture_base(
    const int clientCount,
    const ot::Options& clientArgs)
    : Regtest_fixture_base(clientCount, ot::Options{}, clientArgs)
{
}

auto Regtest_fixture_base::Connect() noexcept -> bool
{
    return Connect(address_);
}

auto Regtest_fixture_base::Connect(const b::p2p::Address& address) noexcept
    -> bool
{
    const auto miner = [&]() -> std::function<bool()> {
        const auto& miner = miner_.Network().Blockchain().GetChain(test_chain_);
        const auto listen = miner.Listen(address);

        EXPECT_TRUE(listen);

        return [=] {
            EXPECT_EQ(connection_.miner_peers_, client_count_);

            return listen && (client_count_ == connection_.miner_peers_);
        };
    }();
    const auto client1 = [&]() -> std::function<bool()> {
        if (0 < client_count_) {
            const auto& client =
                client_1_.Network().Blockchain().GetChain(test_chain_);
            const auto added = client.AddPeer(address);

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
                client_2_.Network().Blockchain().GetChain(test_chain_);
            const auto added = client.AddPeer(address);

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

auto Regtest_fixture_base::get_bytes(const Script& script) noexcept
    -> std::optional<ot::ReadView>
{
    switch (script.Type()) {
        case Pattern::PayToPubkey: {

            return script.Pubkey();
        }
        case Pattern::PayToPubkeyHash: {

            return script.PubkeyHash();
        }
        case Pattern::PayToMultisig: {

            return script.MultisigPubkey(0);
        }
        default: {

            return std::nullopt;
        }
    }
}

auto Regtest_fixture_base::init_address(const ot::api::Core& api) noexcept
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

auto Regtest_fixture_base::init_block(
    const int index,
    const ot::api::Core& api) noexcept -> BlockListener&
{
    auto& p = block_listener_[index];

    if (false == bool(p)) { p = std::make_unique<BlockListener>(api); }

    OT_ASSERT(p);

    return *p;
}

auto Regtest_fixture_base::init_mined() noexcept -> MinedBlocks&
{
    if (false == bool(mined_block_cache_)) {
        mined_block_cache_ = std::make_unique<MinedBlocks>();
    }

    OT_ASSERT(mined_block_cache_);

    return *mined_block_cache_;
}

auto Regtest_fixture_base::init_peer(
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

auto Regtest_fixture_base::init_wallet(
    const int index,
    const ot::api::Core& api) noexcept -> WalletListener&
{
    auto& p = wallet_listener_[index];

    if (false == bool(p)) { p = std::make_unique<WalletListener>(api); }

    OT_ASSERT(p);

    return *p;
}

auto Regtest_fixture_base::Mine(
    const Height ancestor,
    const std::size_t count) noexcept -> bool
{
    return Mine(ancestor, count, default_);
}

auto Regtest_fixture_base::Mine(
    const Height ancestor,
    const std::size_t count,
    const Generator& gen,
    const std::vector<Transaction>& extra) noexcept -> bool
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

    const auto& network = miner_.Network().Blockchain().GetChain(test_chain_);
    const auto& headerOracle = network.HeaderOracle();
    auto previousHeader =
        headerOracle.LoadHeader(headerOracle.BestHash(ancestor))->as_Bitcoin();

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
    constexpr auto limit = std::chrono::minutes(5);
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

auto Regtest_fixture_base::Shutdown() noexcept -> void
{
    wallet_listener_.clear();
    block_listener_.clear();
    mined_block_cache_.reset();
    peer_listener_.reset();
    listen_address_.reset();
}

auto Regtest_fixture_base::Start() noexcept -> bool
{
    const auto miner = [&]() -> std::function<bool()> {
        const auto start = miner_.Network().Blockchain().Start(test_chain_);

        return [=] {
            EXPECT_TRUE(start);

            return start;
        };
    }();
    const auto client1 = [&]() -> std::function<bool()> {
        if (0 < client_count_) {
            const auto start =
                client_1_.Network().Blockchain().Start(test_chain_);

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
            const auto start =
                client_2_.Network().Blockchain().Start(test_chain_);

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

auto Regtest_fixture_base::TestUTXOs(
    const Expected& expected,
    const std::vector<UTXO>& utxos) const noexcept -> bool
{
    auto out = true;

    for (const auto& utxo : utxos) {
        const auto& [outpoint, pOutput] = utxo;

        EXPECT_TRUE(pOutput);

        if (!pOutput) {
            out = false;

            continue;
        }

        const auto& output = *pOutput;

        try {
            const auto& [exKey, exAmount, exPattern] = expected.at(outpoint);
            out &= (output.Value() == exAmount);

            EXPECT_EQ(output.Value(), exAmount);

            const auto& script = output.Script();
            using Position = ot::blockchain::block::bitcoin::Script::Position;
            out &= (script.Role() == Position::Output);

            EXPECT_EQ(script.Role(), Position::Output);

            const auto data = get_bytes(script);

            EXPECT_TRUE(data.has_value());

            if (false == data.has_value()) {
                out = false;

                continue;
            }

            out &= (data.value() == exKey->Bytes());
            out &= (script.Type() == exPattern);

            EXPECT_EQ(data.value(), exKey->Bytes());
            EXPECT_EQ(script.Type(), exPattern);
        } catch (...) {
            EXPECT_EQ(outpoint.str(), "this will never be true");

            out = false;
        }
    }

    return out;
}

Regtest_fixture_normal::Regtest_fixture_normal(const int clientCount)
    : Regtest_fixture_base(
          clientCount,
          ot::Options{}.SetBlockchainStorageLevel(1))
{
}

Regtest_fixture_single::Regtest_fixture_single()
    : Regtest_fixture_normal(1)
{
}

Regtest_fixture_sync::Regtest_fixture_sync()
    : Regtest_fixture_base(
          1,
          ot::Options{}.SetBlockchainStorageLevel(2).SetBlockchainSyncEnabled(
              true))
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
                  sync_requestor_ =
                      std::make_unique<SyncRequestor>(client_1_, mined_blocks_);
              }

              return *sync_requestor_;
          }()

      )
{
}

auto Regtest_fixture_sync::Connect() noexcept -> bool
{
    auto output = Regtest_fixture_base::Connect();
    output &= client_1_.Network().Blockchain().StartSyncServer(
        sync_server_sync_,
        sync_server_sync_public_,
        sync_server_update_,
        sync_server_update_public_);

    return output;
}

auto Regtest_fixture_sync::Shutdown() noexcept -> void
{
    sync_requestor_.reset();
    sync_subscriber_.reset();
    Regtest_fixture_base::Shutdown();
}

Regtest_fixture_tcp::Regtest_fixture_tcp()
    : Regtest_fixture_base(
          1,
          ot::Options{},
          ot::Options{}.SetBlockchainStorageLevel(1))
    , tcp_listen_address_(miner_.Factory().BlockchainAddress(
          b::p2p::Protocol::bitcoin,
          b::p2p::Network::ipv4,
          miner_.Factory().Data("0x7f000001", ot::StringStyle::Hex),
          18444,
          test_chain_,
          {},
          {}))
{
}

auto Regtest_fixture_tcp::Connect() noexcept -> bool
{
    return Connect(tcp_listen_address_);
}

struct ScanListener::Imp {
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
        Data(Data&&) = default;

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

        OT_ASSERT(body.size() == 8u);

        const auto chain = body.at(1).as<Chain>();
        auto nymID = [&] {
            auto out = api_.Factory().NymID();
            out->Assign(body.at(2).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        auto accountID = [&] {
            auto out = api_.Factory().Identifier();
            out->Assign(body.at(4).Bytes());

            OT_ASSERT(false == out->empty());

            return out;
        }();
        const auto sub = body.at(5).as<Subchain>();
        const auto height = body.at(6).as<Height>();
        auto hash = [&] {
            auto out = api_.Factory().Data();
            out->Assign(body.at(7).Bytes());

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

    Imp(const ot::api::Core& api) noexcept
        : api_(api)
        , cb_(Callback::Factory([&](auto& msg) { cb(msg); }))
        , socket_([&] {
            auto out = api_.Network().ZeroMQ().SubscribeSocket(cb_);
            const auto rc =
                out->Start(api_.Endpoints().BlockchainScanProgress());

            OT_ASSERT(rc);

            return out;
        }())
        , lock_()
        , map_()
    {
    }
};

ScanListener::ScanListener(const ot::api::Core& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto ScanListener::get_future(
    const Subaccount& account,
    Subchain subchain,
    Height target) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    const auto& nym = account.Parent().NymID();
    const auto chain = account.Parent().Parent().Chain();
    const auto& id = account.ID();
    auto& map = imp_->map_[nym][chain][id];
    auto it = [&] {
        if (auto i = map.find(subchain); i != map.end()) {

            return i;
        } else {
            return map.try_emplace(subchain, -1, imp_->api_.Factory().Data())
                .first;
        }
    }();

    return it->second.reset(target);
}

auto ScanListener::wait(const Future& future) const noexcept -> bool
{
    constexpr auto limit = std::chrono::minutes{5};
    using Status = std::future_status;

    if (Status::ready == future.wait_for(limit)) {

        return true;
    } else {

        return false;
    }
}

ScanListener::~ScanListener() = default;

struct SyncRequestor::Imp {
    using Buffer = std::deque<ot::OTZMQMessage>;

    SyncRequestor& parent_;
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

    Imp(SyncRequestor& parent,
        const ot::api::client::Manager& api,
        const MinedBlocks& cache) noexcept
        : parent_(parent)
        , api_(api)
        , cache_(cache)
        , lock_()
        , updated_(0)
        , buffer_()
        , cb_(zmq::ListenCallback::Factory([&](auto& msg) { cb(msg); }))
        , socket_(api.Network().ZeroMQ().DealerSocket(cb_, Dir::Connect))
    {
        socket_->Start(sync_server_sync_);
    }
};

SyncRequestor::SyncRequestor(
    const ot::api::client::Manager& api,
    const MinedBlocks& cache) noexcept
    : checked_(-1)
    , expected_(0)
    , imp_(std::make_unique<Imp>(*this, api, cache))
{
}

auto SyncRequestor::check(const State& state, const Position& pos)
    const noexcept -> bool
{
    auto output{true};
    output &= (state.Chain() == test_chain_);
    output &= (state.Position() == pos);

    EXPECT_EQ(state.Chain(), test_chain_);
    EXPECT_EQ(state.Position(), pos);

    return output;
}

auto SyncRequestor::check(const State& state, const std::size_t index) const
    -> bool
{
    auto pos = [&] {
        const auto& chain =
            imp_->api_.Network().Blockchain().GetChain(test_chain_);
        auto header =
            chain.HeaderOracle().LoadHeader(imp_->cache_.get(index).get());

        OT_ASSERT(header);

        return header->Position();
    }();

    return check(state, pos);
}

auto SyncRequestor::check(const otsync::Block& block, const std::size_t index)
    const noexcept -> bool
{
    constexpr auto filterType{ot::blockchain::filter::Type::ES};
    const auto& chain = imp_->api_.Network().Blockchain().GetChain(test_chain_);
    const auto header =
        chain.HeaderOracle().LoadHeader(imp_->cache_.get(index).get());

    OT_ASSERT(header);

    auto headerBytes = ot::Space{};

    EXPECT_TRUE(header->Serialize(ot::writer(headerBytes)));

    const auto& pos = header->Position();
    auto output{true};
    output &= (block.Chain() == test_chain_);
    output &= (block.Height() == pos.first);
    output &= (block.Header() == ot::reader(headerBytes));
    output &= (block.FilterType() == filterType);
    // TODO verify filter

    EXPECT_EQ(block.Chain(), test_chain_);
    EXPECT_EQ(block.Height(), pos.first);
    EXPECT_EQ(block.Header(), ot::reader(headerBytes));
    EXPECT_EQ(block.FilterType(), filterType);

    return output;
}

auto SyncRequestor::get(const std::size_t index) const -> const zmq::Message&
{
    auto lock = ot::Lock{imp_->lock_};

    return imp_->buffer_.at(index);
}

auto SyncRequestor::request(const Position& pos) const noexcept -> bool
{
    auto msg = imp_->api_.Network().ZeroMQ().Message();
    const auto req = otsync::Request{[&] {
        auto out = otsync::Request::StateData{};
        out.emplace_back(test_chain_, pos);

        return out;
    }()};

    if (false == req.Serialize(msg)) {
        EXPECT_TRUE(false);

        return false;
    }

    return imp_->socket_->Send(msg);
}

auto SyncRequestor::wait(const bool hard) noexcept -> bool
{
    const auto limit =
        hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
    auto start = ot::Clock::now();

    while ((imp_->updated_ < expected_) &&
           ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    if (false == hard) { imp_->updated_.store(expected_.load()); }

    return imp_->updated_ >= expected_;
}

SyncRequestor::~SyncRequestor() = default;

struct SyncSubscriber::Imp {
    SyncSubscriber& parent_;
    const ot::api::client::Manager& api_;
    const MinedBlocks& cache_;
    std::atomic_int updated_;
    std::atomic_int errors_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    auto check_update(const ot::network::zeromq::Message& in) noexcept -> void
    {
        namespace sync = ot::network::blockchain::sync;
        const auto base = sync::Factory(api_, in);

        try {
            const auto& data = base->asData();
            const auto& state = data.State();
            const auto& blocks = data.Blocks();
            const auto index = updated_++;
            const auto future = cache_.get(index);
            const auto hash = future.get();

            if (sync::MessageType::new_block_header != base->Type()) {
                throw std::runtime_error{"invalid message"};
            }

            if (state.Chain() != test_chain_) {
                throw std::runtime_error{"wrong chain"};
            }

            if (0 == data.PreviousCfheader().size()) {
                throw std::runtime_error{"invalid previous cfheader"};
            }

            if (0 == blocks.size()) {
                throw std::runtime_error{"no block data"};
            }

            if (1 != blocks.size()) {
                throw std::runtime_error{"wrong number of blocks"};
            }

            if (state.Position().second != hash) {
                std::runtime_error("wrong hash");
            }
        } catch (const std::exception& e) {
            std::cout << e.what() << '\n';
            ++errors_;
        }
    }
    auto wait_for_counter(const bool hard = true) noexcept -> bool
    {
        const auto limit =
            hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
        auto start = ot::Clock::now();
        const auto& expected = parent_.expected_;

        while ((updated_ < expected) && ((ot::Clock::now() - start) < limit)) {
            ot::Sleep(std::chrono::milliseconds(100));
        }

        if (false == hard) { updated_.store(expected.load()); }

        return updated_ >= expected;
    }

    Imp(SyncSubscriber& parent,
        const ot::api::client::Manager& api,
        const MinedBlocks& cache)
        : parent_(parent)
        , api_(api)
        , cache_(cache)
        , updated_(0)
        , errors_(0)
        , cb_(ot::network::zeromq::ListenCallback::Factory(
              [&](const auto& in) { check_update(in); }))
        , socket_(api_.Network().ZeroMQ().SubscribeSocket(cb_))
    {
        if (false == socket_->Start(sync_server_update_)) {
            throw std::runtime_error("Failed to subscribe to updates");
        }
    }
};

SyncSubscriber::SyncSubscriber(
    const ot::api::client::Manager& api,
    const MinedBlocks& cache)
    : expected_(0)
    , imp_(std::make_unique<Imp>(*this, api, cache))
{
}

auto SyncSubscriber::wait(const bool hard) noexcept -> bool
{
    if (imp_->wait_for_counter(hard)) {
        auto output = (0 == imp_->errors_);
        imp_->errors_ = 0;

        return output;
    }

    return false;
}

SyncSubscriber::~SyncSubscriber() = default;

struct WalletListener::Imp {
    const ot::api::Core& api_;
    mutable std::mutex lock_;
    std::promise<Height> promise_;
    Height target_;
    ot::OTZMQListenCallback cb_;
    ot::OTZMQSubscribeSocket socket_;

    Imp(const ot::api::Core& api) noexcept
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
        , socket_(api_.Network().ZeroMQ().SubscribeSocket(cb_))
    {
        OT_ASSERT(socket_->Start(api_.Endpoints().BlockchainSyncProgress()));
    }
};

WalletListener::WalletListener(const ot::api::Core& api) noexcept
    : imp_(std::make_unique<Imp>(api))
{
}

auto WalletListener::GetFuture(const Height height) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    imp_->target_ = height;

    try {
        imp_->promise_ = {};
    } catch (...) {
    }

    return imp_->promise_.get_future();
}

WalletListener::~WalletListener() = default;

std::unique_ptr<const ot::OTBlockchainAddress>
    Regtest_fixture_base::listen_address_{};
std::unique_ptr<const PeerListener> Regtest_fixture_base::peer_listener_{};
std::unique_ptr<MinedBlocks> Regtest_fixture_base::mined_block_cache_{};
Regtest_fixture_base::BlockListen Regtest_fixture_base::block_listener_{};
Regtest_fixture_base::WalletListen Regtest_fixture_base::wallet_listener_{};
std::unique_ptr<SyncSubscriber> Regtest_fixture_sync::sync_subscriber_{};
std::unique_ptr<SyncRequestor> Regtest_fixture_sync::sync_requestor_{};
}  // namespace ottest
