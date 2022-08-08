// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "ottest/Basic.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
class Notary;
}  // namespace session

class Context;
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Outpoint;
}  // namespace block

namespace crypto
{
class Account;
class HD;
class PaymentCode;
class Subaccount;
}  // namespace crypto

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Notary;
class UnitDefinition;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace p2p
{
class Base;
class Block;
class State;
}  // namespace p2p

namespace zeromq
{
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace b = ot::blockchain;
namespace zmq = ot::network::zeromq;
namespace otsync = ot::network::p2p;
namespace bca = ot::blockchain::crypto;

namespace ottest
{
using Position = ot::blockchain::block::Position;
using State = otsync::State;
using Pattern = ot::blockchain::bitcoin::block::Script::Pattern;
using FilterType = ot::blockchain::cfilter::Type;

constexpr auto test_chain_{b::Type::UnitTest};
constexpr auto sync_server_update_public_{
    "inproc://sync_server_public_endpoint/update"};
constexpr auto coinbase_fun_{"The Industrial Revolution and its consequences "
                             "have been a disaster for the human race."};

class User;
struct Server;

class BlockchainStartup
{
public:
    using Future = std::shared_future<void>;

    auto BlockOracle() const noexcept -> Future;
    auto BlockOracleDownloader() const noexcept -> Future;
    auto FeeOracle() const noexcept -> Future;
    auto FilterOracle() const noexcept -> Future;
    auto FilterOracleFilterDownloader() const noexcept -> Future;
    auto FilterOracleHeaderDownloader() const noexcept -> Future;
    auto FilterOracleIndexer() const noexcept -> Future;
    auto Node() const noexcept -> Future;
    auto PeerManager() const noexcept -> Future;
    auto SyncServer() const noexcept -> Future;
    auto Wallet() const noexcept -> Future;

    BlockchainStartup(
        const ot::api::Session& api,
        const ot::blockchain::Type chain) noexcept;

    ~BlockchainStartup();

private:
    class Imp;

    std::unique_ptr<Imp> imp_;
};

class PeerListener
{
    std::promise<void> promise_;

public:
    std::future<void> done_;
    std::atomic_int miner_1_peers_;
    std::atomic_int sync_server_peers_;
    std::atomic_int client_1_peers_;
    std::atomic_int client_2_peers_;

    PeerListener(
        const bool waitForHandshake,
        const int clientCount,
        const ot::api::session::Client& miner,
        const ot::api::session::Client& syncServer,
        const ot::api::session::Client& client1,
        const ot::api::session::Client& client2);

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

    auto GetFuture(const Height height) noexcept -> Future;

    BlockListener(const ot::api::Session& api) noexcept;

    ~BlockListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

class MinedBlocks
{
public:
    using BlockHash = b::block::Hash;
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
    auto request(const otsync::Base& command) const noexcept -> bool;
    auto wait(const bool hard = true) noexcept -> bool;

    SyncRequestor(
        const ot::api::session::Client& api,
        const MinedBlocks& cache) noexcept;

    ~SyncRequestor();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

struct SyncSubscriber {
    using BlockHash = ot::blockchain::block::Hash;
    using SyncPromise = std::promise<BlockHash>;
    using SyncFuture = std::shared_future<BlockHash>;

    std::atomic_int expected_;

    auto wait(const bool hard = true) noexcept -> bool;

    SyncSubscriber(
        const ot::api::session::Client& api,
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

    WalletListener(const ot::api::Session& api) noexcept;

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

    ScanListener(const ot::api::Session& api) noexcept;

    ~ScanListener();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

struct TXOState {
    struct Data {
        ot::blockchain::Balance balance_;
        ot::UnallocatedMap<
            ot::blockchain::node::TxoState,
            ot::UnallocatedSet<ot::blockchain::block::Outpoint>>
            data_;

        Data() noexcept;
    };

    struct NymData {
        Data nym_;
        ot::UnallocatedMap<ot::OTIdentifier, Data> accounts_;

        NymData() noexcept;
    };

    Data wallet_;
    ot::UnallocatedMap<ot::OTNymID, NymData> nyms_;

    TXOState() noexcept;
};

struct TXOs {
    auto AddConfirmed(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool;
    auto AddGenerated(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner,
        const ot::blockchain::block::Height position) noexcept -> bool;
    auto AddUnconfirmed(
        const ot::blockchain::bitcoin::block::Transaction& tx,
        const std::size_t index,
        const ot::blockchain::crypto::Subaccount& owner) noexcept -> bool;
    auto Confirm(const ot::blockchain::block::Txid& transaction) noexcept
        -> bool;
    auto Mature(const ot::blockchain::block::Height position) noexcept -> bool;
    auto Orphan(const ot::blockchain::block::Txid& transaction) noexcept
        -> bool;
    auto OrphanGeneration(
        const ot::blockchain::block::Txid& transaction) noexcept -> bool;
    auto SpendUnconfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool;
    auto SpendConfirmed(const ot::blockchain::block::Outpoint& txo) noexcept
        -> bool;

    auto Extract(TXOState& output) const noexcept -> void;

    TXOs(const User& owner) noexcept;
    TXOs() = delete;
    TXOs(const TXOs&) = delete;
    TXOs(TXOs&&) = delete;
    auto operator=(const TXOs&) -> TXOs& = delete;
    auto operator=(TXOs&&) -> TXOs& = delete;

    ~TXOs();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};

class Regtest_fixture_base : virtual public ::testing::Test
{
public:
    static auto MaturationInterval() noexcept -> ot::blockchain::block::Height;

protected:
    using Height = b::block::Height;
    using Transaction = ot::api::session::Factory::Transaction_p;
    using Transactions = ot::UnallocatedDeque<ot::blockchain::block::pTxid>;
    using Generator = std::function<Transaction(Height)>;
    using Outpoint = ot::blockchain::block::Outpoint;
    using Script = ot::blockchain::bitcoin::block::Script;
    using UTXO = ot::blockchain::node::Wallet::UTXO;
    using Key = ot::OTData;
    using Amount = ot::Amount;
    using OutpointMetadata = std::tuple<Key, Amount, Pattern>;
    using Expected = ot::UnallocatedMap<Outpoint, OutpointMetadata>;
    using Subchain = bca::Subchain;

    static bool init_;
    static Expected expected_;
    static Transactions transactions_ptxid_;
    static ot::blockchain::block::Height height_;
    static std::optional<BlockchainStartup> miner_startup_s_;
    static std::optional<BlockchainStartup> sync_server_startup_s_;
    static std::optional<BlockchainStartup> client_1_startup_s_;
    static std::optional<BlockchainStartup> client_2_startup_s_;

    const ot::api::Context& ot_;
    const ot::Options client_args_;
    const int client_count_;
    const ot::api::session::Client& miner_;
    const ot::api::session::Client& sync_server_;
    const ot::api::session::Client& client_1_;
    const ot::api::session::Client& client_2_;
    const BlockchainStartup& miner_startup_;
    const BlockchainStartup& sync_server_startup_;
    const BlockchainStartup& client_1_startup_;
    const BlockchainStartup& client_2_startup_;
    const b::p2p::Address& address_;
    const PeerListener& connection_;
    const Generator default_;
    MinedBlocks& mined_blocks_;
    BlockListener& block_1_;
    BlockListener& block_2_;
    WalletListener& wallet_1_;
    WalletListener& wallet_2_;

    auto Account(const User& user, ot::blockchain::Type chain) noexcept
        -> const bca::Account&;
    virtual auto Connect() noexcept -> bool;
    auto Connect(const b::p2p::Address& address) noexcept -> bool;
    auto Mine(const Height ancestor, const std::size_t count) noexcept -> bool;
    auto Mine(
        const Height ancestor,
        const std::size_t count,
        const Generator& gen,
        const ot::UnallocatedVector<Transaction>& extra = {}) noexcept -> bool;
    auto TestUTXOs(const Expected& expected, const ot::Vector<UTXO>& utxos)
        const noexcept -> bool;
    auto TestWallet(const ot::api::session::Client& api, const TXOState& state)
        const noexcept -> bool;

    virtual auto Shutdown() noexcept -> void;
    auto Start() noexcept -> bool;

    Regtest_fixture_base(
        const bool waitForHandshake,
        const int clientCount,
        const ot::Options& clientArgs);
    Regtest_fixture_base(
        const bool waitForHandshake,
        const int clientCount,
        const ot::Options& minerArgs,
        const ot::Options& clientArgs);

private:
    using BlockListen = ot::UnallocatedMap<int, std::unique_ptr<BlockListener>>;
    using WalletListen =
        ot::UnallocatedMap<int, std::unique_ptr<WalletListener>>;

    static const ot::UnallocatedSet<ot::blockchain::node::TxoState> states_;
    static std::unique_ptr<const ot::OTBlockchainAddress> listen_address_;
    static std::unique_ptr<const PeerListener> peer_listener_;
    static std::unique_ptr<MinedBlocks> mined_block_cache_;
    static BlockListen block_listener_;
    static WalletListen wallet_listener_;

    static auto get_bytes(const Script& script) noexcept
        -> std::optional<ot::ReadView>;
    static auto init_address(const ot::api::Session& api) noexcept
        -> const b::p2p::Address&;
    static auto init_block(
        const int index,
        const ot::api::Session& api) noexcept -> BlockListener&;
    static auto init_mined() noexcept -> MinedBlocks&;
    static auto init_peer(
        const bool waitForHandshake,
        const int clientCount,
        const ot::api::session::Client& miner,
        const ot::api::session::Client& syncServer,
        const ot::api::session::Client& client1,
        const ot::api::session::Client& client2) noexcept
        -> const PeerListener&;
    static auto init_wallet(
        const int index,
        const ot::api::Session& api) noexcept -> WalletListener&;

    auto compare_outpoints(
        const ot::blockchain::node::Wallet& wallet,
        const TXOState::Data& data) const noexcept -> bool;
    auto compare_outpoints(
        const ot::blockchain::node::Wallet& wallet,
        const ot::identifier::Nym& nym,
        const TXOState::Data& data) const noexcept -> bool;
    auto compare_outpoints(
        const ot::blockchain::node::Wallet& wallet,
        const ot::identifier::Nym& nym,
        const ot::Identifier& subaccount,
        const TXOState::Data& data) const noexcept -> bool;
    auto compare_outpoints(
        const ot::blockchain::node::TxoState type,
        const TXOState::Data& expected,
        const ot::Vector<UTXO>& got) const noexcept -> bool;
};

class Regtest_fixture_normal : public Regtest_fixture_base
{
protected:
    Regtest_fixture_normal(const int clientCount);
    Regtest_fixture_normal(
        const int clientCount,
        const ot::Options& clientArgs);
};

class Regtest_fixture_hd : public Regtest_fixture_normal
{
protected:
    static User alice_;
    static TXOs txos_;
    static std::unique_ptr<ScanListener> listener_p_;

    const ot::identifier::Notary& expected_notary_;
    const ot::identifier::UnitDefinition& expected_unit_;
    const ot::UnallocatedCString expected_display_unit_;
    const ot::UnallocatedCString expected_account_name_;
    const ot::UnallocatedCString expected_notary_name_;
    const ot::UnallocatedCString memo_outgoing_;
    const ot::AccountType expected_account_type_;
    const ot::UnitType expected_unit_type_;
    const Generator hd_generator_;
    ScanListener& listener_;

    auto CheckTXODB() const noexcept -> bool;
    auto SendHD() const noexcept -> const bca::HD&;

    auto Shutdown() noexcept -> void final;

    Regtest_fixture_hd();
};

class Regtest_payment_code : public Regtest_fixture_normal
{
protected:
    static constexpr auto message_text_{
        "I have come here to chew bubblegum and kick ass...and I'm all out of "
        "bubblegum."};

    static User alice_;
    static User bob_;
    static Server server_1_;
    static TXOs txos_alice_;
    static TXOs txos_bob_;
    static std::unique_ptr<ScanListener> listener_alice_p_;
    static std::unique_ptr<ScanListener> listener_bob_p_;

    const ot::api::session::Notary& api_server_1_;
    const ot::identifier::Notary& expected_notary_;
    const ot::identifier::UnitDefinition& expected_unit_;
    const ot::UnallocatedCString expected_display_unit_;
    const ot::UnallocatedCString expected_account_name_;
    const ot::UnallocatedCString expected_notary_name_;
    const ot::UnallocatedCString memo_outgoing_;
    const ot::AccountType expected_account_type_;
    const ot::UnitType expected_unit_type_;
    const Generator mine_to_alice_;
    ScanListener& listener_alice_;
    ScanListener& listener_bob_;

    auto CheckContactID(
        const User& local,
        const User& remote,
        const ot::UnallocatedCString& paymentcode) const noexcept -> bool;
    auto CheckTXODBAlice() const noexcept -> bool;
    auto CheckTXODBBob() const noexcept -> bool;

    auto ReceiveHD() const noexcept -> const bca::HD&;
    auto ReceivePC() const noexcept -> const bca::PaymentCode&;
    auto SendHD() const noexcept -> const bca::HD&;
    auto SendPC() const noexcept -> const bca::PaymentCode&;

    auto Shutdown() noexcept -> void final;

    Regtest_payment_code();
};

class Regtest_fixture_single : public Regtest_fixture_normal
{
protected:
    Regtest_fixture_single();
    Regtest_fixture_single(const ot::Options& clientArgs);
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

class Regtest_fixture_sync : public Regtest_fixture_single
{
protected:
    static User alex_;
    static std::optional<ot::OTServerContract> notary_;
    static std::optional<ot::OTUnitDefinition> unit_;

    SyncSubscriber& sync_sub_;
    SyncRequestor& sync_req_;

    auto Shutdown() noexcept -> void final;

    Regtest_fixture_sync();

private:
    static std::unique_ptr<SyncSubscriber> sync_subscriber_;
    static std::unique_ptr<SyncRequestor> sync_requestor_;
};
}  // namespace ottest
