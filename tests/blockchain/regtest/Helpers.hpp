// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/blockchain/FilterType.hpp"
// IWYU pragma: no_include "opentxs/network/zeromq/ListenCallback.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subaccount.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/Outpoint.hpp"
// IWYU pragma: no_include "opentxs/network/blockchain/sync/State.hpp"
// IWYU pragma: no_include "opentxs/blockchain/FilterType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/FilterType.hpp"
// IWYU pragma: no_include "opentxs/network/zeromq/Context.hpp"

#pragma once

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
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
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "Basic.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Options.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

class Context;
class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
struct Outpoint;
}  // namespace block

namespace crypto
{
class Subaccount;
}  // namespace crypto

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace blockchain
{
namespace sync
{
class Block;
class State;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
}  // namespace opentxs

namespace b = ot::blockchain;
namespace zmq = ot::network::zeromq;
namespace otsync = ot::network::blockchain::sync;

namespace ottest
{
using Position = ot::blockchain::block::Position;
using State = otsync::State;
using Pattern = ot::blockchain::block::bitcoin::Script::Pattern;
using FilterType = ot::blockchain::filter::Type;

constexpr auto test_chain_{b::Type::UnitTest};
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
        const ot::api::client::Manager& client2);

    ~PeerListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

class BlockListener
{
public:
    using Height = b::block::Height;
    using Position = b::block::Position;
    using Future = std::future<Position>;

    auto GetFuture(const Height height) noexcept;

    BlockListener(const ot::api::Core& api) noexcept;

    ~BlockListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

class MinedBlocks
{
public:
    using BlockHash = b::block::pHash;
    using Promise = std::promise<BlockHash>;
    using Future = std::shared_future<BlockHash>;

    auto get(const std::size_t index) const -> Future;

    auto allocate() noexcept -> Promise;

    MinedBlocks();

    ~MinedBlocks();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

struct SyncRequestor {
    std::atomic_int checked_;
    std::atomic_int expected_;

    auto check(const State& state, const Position& pos) const noexcept -> bool;
    auto check(const State& state, const std::size_t index) const -> bool;
    auto check(const otsync::Block& block, const std::size_t index)
        const noexcept -> bool;

    auto get(const std::size_t index) const -> const zmq::Message&;
    auto request(const Position& pos) const noexcept -> bool;
    auto wait(const bool hard = true) noexcept -> bool;

    SyncRequestor(
        const ot::api::client::Manager& api,
        const MinedBlocks& cache) noexcept;

    ~SyncRequestor();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

struct SyncSubscriber {
    using BlockHash = ot::blockchain::block::pHash;
    using SyncPromise = std::promise<BlockHash>;
    using SyncFuture = std::shared_future<BlockHash>;

    std::atomic_int expected_;

    auto wait(const bool hard = true) noexcept -> bool;

    SyncSubscriber(
        const ot::api::client::Manager& api,
        const MinedBlocks& cache);

    ~SyncSubscriber();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

class WalletListener
{
public:
    using Height = b::block::Height;
    using Future = std::future<Height>;

    auto GetFuture(const Height height) noexcept -> Future;

    WalletListener(const ot::api::Core& api) noexcept;

    ~WalletListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

struct ScanListener {
    using Callback = ot::network::zeromq::ListenCallback;
    using Subaccount = ot::blockchain::crypto::Subaccount;
    using Subchain = ot::blockchain::crypto::Subchain;
    using Height = ot::blockchain::block::Height;
    using Future = std::future<void>;

    auto wait(const Future& future) const noexcept -> bool;

    auto get_future(
        const Subaccount& account,
        Subchain subchain,
        Height target) noexcept -> Future;

    ScanListener(const ot::api::Core& api) noexcept;

    ~ScanListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

class Regtest_fixture_base : virtual public ::testing::Test
{
protected:
    using Height = b::block::Height;
    using Transaction = ot::api::Factory::Transaction_p;
    using Generator = std::function<Transaction(Height)>;
    using Outpoint = ot::blockchain::block::Outpoint;
    using Script = ot::blockchain::block::bitcoin::Script;
    using UTXO = ot::blockchain::node::Wallet::UTXO;
    using Key = ot::OTData;
    using Amount = std::int64_t;
    using OutpointMetadata = std::tuple<Key, Amount, Pattern>;
    using Expected = std::map<Outpoint, OutpointMetadata>;

    const ot::api::Context& ot_;
    const ot::Options client_args_;
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

    virtual auto Connect() noexcept -> bool;
    auto Connect(const b::p2p::Address& address) noexcept -> bool;
    auto Mine(const Height ancestor, const std::size_t count) noexcept -> bool;
    auto Mine(
        const Height ancestor,
        const std::size_t count,
        const Generator& gen,
        const std::vector<Transaction>& extra = {}) noexcept -> bool;
    auto TestUTXOs(const Expected& expected, const std::vector<UTXO>& utxos)
        const noexcept -> bool;

    virtual auto Shutdown() noexcept -> void;
    auto Start() noexcept -> bool;

    Regtest_fixture_base(const int clientCount, const ot::Options& clientArgs);
    Regtest_fixture_base(
        const int clientCount,
        const ot::Options& minerArgs,
        const ot::Options& clientArgs);

private:
    using BlockListen = std::map<int, std::unique_ptr<BlockListener>>;
    using WalletListen = std::map<int, std::unique_ptr<WalletListener>>;

    static std::unique_ptr<const ot::OTBlockchainAddress> listen_address_;
    static std::unique_ptr<const PeerListener> peer_listener_;
    static std::unique_ptr<MinedBlocks> mined_block_cache_;
    static BlockListen block_listener_;
    static WalletListen wallet_listener_;

    static auto get_bytes(const Script& script) noexcept
        -> std::optional<ot::ReadView>;
    static auto init_address(const ot::api::Core& api) noexcept
        -> const b::p2p::Address&;
    static auto init_block(const int index, const ot::api::Core& api) noexcept
        -> BlockListener&;
    static auto init_mined() noexcept -> MinedBlocks&;
    static auto init_peer(
        const int clientCount,
        const ot::api::client::Manager& miner,
        const ot::api::client::Manager& client1,
        const ot::api::client::Manager& client2) noexcept
        -> const PeerListener&;
    static auto init_wallet(const int index, const ot::api::Core& api) noexcept
        -> WalletListener&;
};

class Regtest_fixture_normal : public Regtest_fixture_base
{
protected:
    Regtest_fixture_normal(const int clientCount);
};

class Regtest_fixture_single : public Regtest_fixture_normal
{
protected:
    Regtest_fixture_single();
};

class Regtest_fixture_tcp : public Regtest_fixture_base
{
protected:
    using Regtest_fixture_base::Connect;
    auto Connect() noexcept -> bool final;

    Regtest_fixture_tcp();

private:
    const ot::OTBlockchainAddress tcp_listen_address_;
};

class Regtest_fixture_sync : public Regtest_fixture_base
{
protected:
    SyncSubscriber& sync_sub_;
    SyncRequestor& sync_req_;

    using Regtest_fixture_base::Connect;
    auto Connect() noexcept -> bool final;

    auto Shutdown() noexcept -> void final;

    Regtest_fixture_sync();

private:
    static std::unique_ptr<SyncSubscriber> sync_subscriber_;
    static std::unique_ptr<SyncRequestor> sync_requestor_;
};
}  // namespace ottest
