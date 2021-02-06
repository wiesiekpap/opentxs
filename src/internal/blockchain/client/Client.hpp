// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#if OT_BLOCKCHAIN
#include "blockchain/DownloadTask.hpp"
#endif  // OT_BLOCKCHAIN
#include "internal/core/Core.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/Bytes.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Network.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/blockchain/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/blockchain/client/FilterOracle.hpp"
#include "opentxs/blockchain/client/HeaderOracle.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/Data.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/core/Identifier.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/crypto/Types.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/protobuf/BlockchainP2PSync.pb.h"
#include "opentxs/util/WorkType.hpp"
#include "util/Blank.hpp"
#include "util/Work.hpp"
#endif  // OT_BLOCKCHAIN

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace database
{
namespace implementation
{
class Database;
}  // namespace implementation
}  // namespace database
}  // namespace blockchain

namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal

class Core;
}  // namespace api
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
class Header;
class Transaction;
struct Outpoint;
}  // namespace bitcoin

class Block;
class Header;
}  // namespace block

namespace client
{
class UpdateTransaction;
}  // namespace client

namespace internal
{
struct Database;
}  // namespace internal

namespace p2p
{
namespace internal
{
struct Address;
}  // namespace internal

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Frame;
class Message;

namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionOutput;
class BlockchainTransactionProposal;
}  // namespace proto

template <typename T>
struct make_blank;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

#if OT_BLOCKCHAIN
namespace opentxs
{
template <>
struct make_blank<blockchain::block::Height> {
    static auto value(const api::Core&) -> blockchain::block::Height
    {
        return -1;
    }
};
template <>
struct make_blank<blockchain::block::Position> {
    static auto value(const api::Core& api) -> blockchain::block::Position
    {
        return {
            make_blank<blockchain::block::Height>::value(api),
            make_blank<blockchain::block::pHash>::value(api)};
    }
};
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
using BlockReader = ProtectedView<ReadView, std::shared_mutex, sLock>;
using BlockWriter = ProtectedView<WritableView, std::shared_mutex, eLock>;

enum class BlockStorage : std::uint8_t {
    None = 0,
    Cache = 1,
    All = 2,
};
}  // namespace opentxs::api::client::blockchain

namespace opentxs::blockchain::client
{
constexpr auto sync_hello_version_ = VersionNumber{1};
constexpr auto sync_state_version_ = VersionNumber{1};
constexpr auto sync_data_version_ = VersionNumber{1};

// parent hash, child hash
using ChainSegment = std::pair<block::pHash, block::pHash>;
using UpdatedHeader =
    std::map<block::pHash, std::pair<std::unique_ptr<block::Header>, bool>>;
using BestHashes = std::map<block::Height, block::pHash>;
using Hashes = std::set<block::pHash>;
using HashVector = std::vector<block::pHash>;
using Segments = std::set<ChainSegment>;
// parent block hash, disconnected block hash
using DisconnectedList = std::multimap<block::pHash, block::pHash>;

using CfheaderJob =
    download::Batch<filter::pHash, filter::pHeader, filter::Type>;
using CfilterJob =
    download::Batch<std::unique_ptr<const GCS>, filter::pHeader, filter::Type>;
using BlockJob =
    download::Batch<std::shared_ptr<const block::bitcoin::Block>, int>;

using ParsedSyncPacket = std::
    tuple<block::pHash, block::Height, filter::Type, std::uint32_t, OTData>;
using ParsedSyncData = std::pair<block::pHash, std::vector<ParsedSyncPacket>>;
}  // namespace opentxs::blockchain::client
#endif  // OT_BLOCKCHAIN

namespace opentxs::blockchain::client::internal
{
#if OT_BLOCKCHAIN
struct BlockDatabase {
    virtual auto BlockExists(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const block::bitcoin::Block> = 0;
    virtual auto BlockPolicy() const noexcept
        -> api::client::blockchain::BlockStorage = 0;
    virtual auto BlockStore(const block::Block& block) const noexcept
        -> bool = 0;
    virtual auto BlockTip() const noexcept -> block::Position = 0;
    virtual auto SetBlockTip(const block::Position& position) const noexcept
        -> bool = 0;

    virtual ~BlockDatabase() = default;
};

struct BlockOracle : virtual public opentxs::blockchain::client::BlockOracle {
    enum class Task : OTZMQWorkType {
        ProcessBlock = OT_ZMQ_INTERNAL_SIGNAL + 0,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = value(WorkType::Shutdown),
    };

    virtual auto GetBlockJob() const noexcept -> BlockJob = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto SubmitBlock(const ReadView in) const noexcept -> void = 0;
    virtual auto Tip() const noexcept -> block::Position = 0;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~BlockOracle() override = default;
};

struct OPENTXS_EXPORT Config {
    bool download_cfilters_{false};
    bool generate_cfilters_{false};
    bool provide_sync_server_{false};
    bool use_sync_server_{false};
    bool disable_wallet_{false};
    std::string sync_endpoint_{};

    auto print() const noexcept -> std::string;
};

struct FilterDatabase {
    using Hash = block::pHash;
    /// block hash, filter header, filter hash
    using Header = std::tuple<block::pHash, filter::pHeader, ReadView>;
    /// block hash, filter
    using Filter = std::pair<ReadView, std::unique_ptr<const GCS>>;

    virtual auto FilterHeaderTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto FilterTip(const filter::Type type) const noexcept
        -> block::Position = 0;
    virtual auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool = 0;
    virtual auto HaveFilterHeader(
        const filter::Type type,
        const block::Hash& block) const noexcept -> bool = 0;
    virtual auto LoadFilter(const filter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const GCS> = 0;
    virtual auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash = 0;
    virtual auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash = 0;
    virtual auto SetFilterHeaderTip(
        const filter::Type type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto SetFilterTip(
        const filter::Type type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto StoreFilters(
        const filter::Type type,
        std::vector<Filter> filters) const noexcept -> bool = 0;
    virtual auto StoreFilters(
        const filter::Type type,
        const std::vector<Header>& headers,
        const std::vector<Filter>& filters,
        const block::Position& tip) const noexcept -> bool = 0;
    virtual auto StoreFilterHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::vector<Header> headers) const noexcept -> bool = 0;

    virtual ~FilterDatabase() = default;
};

struct FilterOracle : virtual public opentxs::blockchain::client::FilterOracle {
    using Header = FilterDatabase::Hash;

    static auto ProcessThreadPool(const zmq::Message& task) noexcept -> void;

    virtual auto GetFilterJob() const noexcept -> CfilterJob = 0;
    virtual auto GetHeaderJob() const noexcept -> CfheaderJob = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto LoadFilterOrResetTip(
        const filter::Type type,
        const block::Position& position) const noexcept
        -> std::unique_ptr<const GCS> = 0;
    virtual auto ProcessBlock(const block::bitcoin::Block& block) const noexcept
        -> bool = 0;
    virtual auto ProcessSyncData(const ParsedSyncData& data) const noexcept
        -> void = 0;
    virtual auto Tip(const filter::Type type) const noexcept
        -> block::Position = 0;

    virtual auto Start() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> void = 0;

    ~FilterOracle() override = default;
};

struct HeaderOracle : virtual public opentxs::blockchain::client::HeaderOracle {
    using CheckpointBlockHash = block::pHash;
    using PreviousBlockHash = block::pHash;
    using CheckpointFilterHash = block::pHash;
    using CheckpointData = std::tuple<
        block::Height,
        CheckpointBlockHash,
        PreviousBlockHash,
        CheckpointFilterHash>;

    virtual auto GetDefaultCheckpoint() const noexcept -> CheckpointData = 0;
    virtual auto Init() noexcept -> void = 0;
    virtual auto LoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> = 0;
    virtual auto ProcessSyncData(
        const network::zeromq::Message& work,
        ParsedSyncData& out) noexcept -> bool = 0;

    ~HeaderOracle() override = default;
};

struct HeaderDatabase {
    virtual auto ApplyUpdate(
        const client::UpdateTransaction& update) const noexcept -> bool = 0;
    // Throws std::out_of_range if no block at that position
    virtual auto BestBlock(const block::Height position) const noexcept(false)
        -> block::pHash = 0;
    virtual auto CurrentBest() const noexcept
        -> std::unique_ptr<block::Header> = 0;
    virtual auto CurrentCheckpoint() const noexcept -> block::Position = 0;
    virtual auto DisconnectedHashes() const noexcept -> DisconnectedList = 0;
    virtual auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto HaveCheckpoint() const noexcept -> bool = 0;
    virtual auto HeaderExists(const block::Hash& hash) const noexcept
        -> bool = 0;
    virtual auto IsSibling(const block::Hash& hash) const noexcept -> bool = 0;
    // Throws std::out_of_range if the header does not exist
    virtual auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> = 0;
    virtual auto RecentHashes() const noexcept -> HashVector = 0;
    virtual auto SiblingHashes() const noexcept -> Hashes = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> = 0;
    // Returns null pointer if the header does not exist
    virtual auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> = 0;

    virtual ~HeaderDatabase() = default;
};

struct IO {
    using tcp = boost::asio::ip::tcp;

    operator boost::asio::io_context&() const noexcept { return context_; }

    auto Connect(
        const Space& id,
        const tcp::endpoint& endpoint,
        tcp::socket& socket) const noexcept -> void;
    auto Receive(
        const Space& id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        tcp::socket& socket) const noexcept -> void;

    auto AddNetwork() noexcept -> void;
    auto Shutdown() noexcept -> void;

    IO(const api::Core& api) noexcept;
    ~IO();

private:
    const api::Core& api_;
    mutable std::mutex lock_;
    OTZMQListenCallback cb_;
    OTZMQRouterSocket socket_;
    mutable int next_buffer_;
    mutable std::map<int, Space> buffers_;
    mutable boost::asio::io_context context_;
    std::unique_ptr<boost::asio::io_context::work> work_;
    boost::thread_group thread_pool_;

    auto clear_buffer(const int id) const noexcept -> void;
    auto get_buffer(const std::size_t bytes) const noexcept
        -> std::pair<int, WritableView>;

    auto callback(zmq::Message& in) noexcept -> void;

    IO() = delete;
    IO(const IO&) = delete;
    IO(IO&&) = delete;
    auto operator=(const IO&) -> IO& = delete;
    auto operator=(IO&&) -> IO& = delete;
};

struct PeerDatabase {
    using Address = std::unique_ptr<p2p::internal::Address>;
    using Protocol = p2p::Protocol;
    using Service = p2p::Service;
    using Type = p2p::Network;

    virtual auto AddOrUpdate(Address address) const noexcept -> bool = 0;
    virtual auto Get(
        const Protocol protocol,
        const std::set<Type> onNetworks,
        const std::set<Service> withServices) const noexcept -> Address = 0;
    virtual auto Import(std::vector<Address> peers) const noexcept -> bool = 0;

    virtual ~PeerDatabase() = default;
};

struct PeerManager {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        Getheaders = OT_ZMQ_INTERNAL_SIGNAL + 0,
        Getblock = OT_ZMQ_INTERNAL_SIGNAL + 1,
        BroadcastTransaction = OT_ZMQ_INTERNAL_SIGNAL + 2,
        BroadcastBlock = OT_ZMQ_INTERNAL_SIGNAL + 3,
        JobAvailableCfheaders = OT_ZMQ_INTERNAL_SIGNAL + 4,
        JobAvailableCfilters = OT_ZMQ_INTERNAL_SIGNAL + 5,
        JobAvailableBlock = OT_ZMQ_INTERNAL_SIGNAL + 6,
        Body = OT_ZMQ_INTERNAL_SIGNAL + 126,
        Header = OT_ZMQ_INTERNAL_SIGNAL + 127,
        Heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        Connect = OT_ZMQ_CONNECT_SIGNAL,
        Disconnect = OT_ZMQ_DISCONNECT_SIGNAL,
        ReceiveMessage = OT_ZMQ_RECEIVE_SIGNAL,
        SendMessage = OT_ZMQ_SEND_SIGNAL,
        Register = OT_ZMQ_REGISTER_SIGNAL,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    virtual auto AddIncomingPeer(const int id, std::uintptr_t endpoint)
        const noexcept -> void = 0;
    virtual auto AddPeer(const p2p::Address& address) const noexcept
        -> bool = 0;
    virtual auto BroadcastBlock(const block::Block& block) const noexcept
        -> bool = 0;
    virtual auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool = 0;
    virtual auto Connect() noexcept -> bool = 0;
    virtual auto Database() const noexcept -> const PeerDatabase& = 0;
    virtual auto Disconnect(const int id) const noexcept -> void = 0;
    virtual auto Endpoint(const Task type) const noexcept -> std::string = 0;
    virtual auto GetPeerCount() const noexcept -> std::size_t = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto JobReady(const Task type) const noexcept -> void = 0;
    virtual auto Listen(const p2p::Address& address) const noexcept -> bool = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestBlocks(
        const std::vector<ReadView>& hashes) const noexcept -> bool = 0;
    virtual auto RequestHeaders() const noexcept -> bool = 0;

    virtual auto init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~PeerManager() = default;
};

struct Network : virtual public opentxs::blockchain::Network {
    enum class Task : OTZMQWorkType {
        Shutdown = value(WorkType::Shutdown),
        SubmitBlockHeader = OT_ZMQ_INTERNAL_SIGNAL + 0,
        SubmitBlock = OT_ZMQ_INTERNAL_SIGNAL + 2,
        Heartbeat = OT_ZMQ_INTERNAL_SIGNAL + 3,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        FilterUpdate = OT_ZMQ_NEW_FILTER_SIGNAL,
        SyncData = OT_ZMQ_SYNC_DATA_SIGNAL,
    };

    virtual auto Blockchain() const noexcept
        -> const api::client::internal::Blockchain& = 0;
    virtual auto BlockOracle() const noexcept
        -> const internal::BlockOracle& = 0;
    virtual auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool = 0;
    virtual auto Chain() const noexcept -> Type = 0;
    // amount represents satoshis per 1000 bytes
    virtual auto FeeRate() const noexcept -> Amount = 0;
    auto FilterOracle() const noexcept -> const client::FilterOracle& final
    {
        return FilterOracleInternal();
    }
    virtual auto FilterOracleInternal() const noexcept
        -> const internal::FilterOracle& = 0;
    auto HeaderOracle() const noexcept -> const client::HeaderOracle& final
    {
        return HeaderOracleInternal();
    }
    virtual auto HeaderOracleInternal() const noexcept
        -> const internal::HeaderOracle& = 0;
    virtual auto Heartbeat() const noexcept -> void = 0;
    virtual auto IsSynchronized() const noexcept -> bool = 0;
    virtual auto JobReady(const PeerManager::Task type) const noexcept
        -> void = 0;
    virtual auto Reorg() const noexcept
        -> const network::zeromq::socket::Publish& = 0;
    virtual auto RequestBlock(const block::Hash& block) const noexcept
        -> bool = 0;
    virtual auto RequestBlocks(
        const std::vector<ReadView>& hashes) const noexcept -> bool = 0;
    virtual auto Submit(network::zeromq::Message& work) const noexcept
        -> void = 0;
    virtual auto UpdateHeight(const block::Height height) const noexcept
        -> void = 0;
    virtual auto UpdateLocalHeight(
        const block::Position position) const noexcept -> void = 0;

    virtual auto HeaderOracleInternal() noexcept -> internal::HeaderOracle& = 0;
    virtual auto FilterOracleInternal() noexcept -> internal::FilterOracle& = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    ~Network() override = default;
};

struct SyncDatabase {
    using Height = block::Height;
    using Items = std::vector<proto::BlockchainP2PSync>;
    using Message = network::zeromq::Message;

    virtual auto LoadSync(const Height height, Message& output) const noexcept
        -> bool = 0;
    virtual auto ReorgSync(const Height height) const noexcept -> bool = 0;
    virtual auto SetSyncTip(const block::Position& position) const noexcept
        -> bool = 0;
    virtual auto StoreSync(const block::Position& tip, const Items& items)
        const noexcept -> bool = 0;
    virtual auto SyncTip() const noexcept -> block::Position = 0;

    virtual ~SyncDatabase() = default;
};

struct ThreadPool {
    using Future = std::shared_future<void>;

    enum class Work : OTZMQWorkType {
        HDAccount = OT_ZMQ_INTERNAL_SIGNAL + 0,
        SyncDataFiltersIncoming = OT_ZMQ_INTERNAL_SIGNAL + 1,
    };

    static auto Capacity() noexcept -> std::size_t;
    static auto MakeWork(
        const api::Core& api,
        const blockchain::Type chain,
        const Work type) noexcept -> OTZMQMessage;

    virtual auto Endpoint() const noexcept -> std::string = 0;
    virtual auto Reset(const Type chain) const noexcept -> void = 0;
    virtual auto Stop(const Type chain) const noexcept -> Future = 0;

    virtual ~ThreadPool() = default;
};

struct Wallet {
    enum class Task : OTZMQWorkType {
        index = OT_ZMQ_INTERNAL_SIGNAL + 0,
        scan = OT_ZMQ_INTERNAL_SIGNAL + 1,
        process = OT_ZMQ_INTERNAL_SIGNAL + 2,
        reorg = OT_ZMQ_INTERNAL_SIGNAL + 3,
    };

    static auto ProcessThreadPool(const zmq::Message& task) noexcept -> void;

    virtual auto ConstructTransaction(
        const proto::BlockchainTransactionProposal& tx) const noexcept
        -> std::future<block::pTxid> = 0;

    virtual auto Init() noexcept -> void = 0;
    virtual auto Shutdown() noexcept -> std::shared_future<void> = 0;

    virtual ~Wallet() = default;
};

struct WalletDatabase {
    using FilterType = filter::Type;
    using NodeID = Identifier;
    using pNodeID = OTIdentifier;
    using Subchain = api::client::blockchain::Subchain;
    using SubchainID = std::pair<Subchain, pNodeID>;
    using ElementID = std::pair<Bip32Index, SubchainID>;
    using ElementMap = std::map<Bip32Index, std::vector<Space>>;
    using Pattern = std::pair<ElementID, Space>;
    using Patterns = std::vector<Pattern>;
    using MatchingIndices = std::vector<Bip32Index>;
    using UTXO = std::pair<
        blockchain::block::bitcoin::Outpoint,
        proto::BlockchainTransactionOutput>;
    using KeyID = api::client::blockchain::Key;

    static const VersionNumber DefaultIndexVersion;

    virtual auto AddConfirmedTransaction(
        const blockchain::Type chain,
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& block,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> bool = 0;
    virtual auto AddOutgoingTransaction(
        const blockchain::Type chain,
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool = 0;
    virtual auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept
        -> bool = 0;
    virtual auto CancelProposal(const Identifier& id) const noexcept
        -> bool = 0;
    virtual auto CompletedProposals() const noexcept
        -> std::set<OTIdentifier> = 0;
    virtual auto DeleteProposal(const Identifier& id) const noexcept
        -> bool = 0;
    virtual auto ForgetProposals(
        const std::set<OTIdentifier>& ids) const noexcept -> bool = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> Patterns = 0;
    virtual auto GetUnspentOutputs() const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetUntestedPatterns(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ReadView blockID,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> Patterns = 0;
    virtual auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal> = 0;
    virtual auto LoadProposals() const noexcept
        -> std::vector<proto::BlockchainTransactionProposal> = 0;
    virtual auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier> = 0;
    virtual auto ReleaseChangeKey(const Identifier& proposal, const KeyID key)
        const noexcept -> bool = 0;
    virtual auto ReorgTo(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const std::vector<block::Position>& reorg) const noexcept -> bool = 0;
    virtual auto ReserveChangeKey(const Identifier& proposal) const noexcept
        -> std::optional<KeyID> = 0;
    virtual auto ReserveUTXO(const Identifier& proposal) const noexcept
        -> std::optional<UTXO> = 0;
    virtual auto SetDefaultFilterType(const FilterType type) const noexcept
        -> bool = 0;
    virtual auto SubchainAddElements(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const ElementMap& elements,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> bool = 0;
    virtual auto SubchainDropIndex(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version) const noexcept -> bool = 0;
    virtual auto SubchainIndexVersion(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> VersionNumber = 0;
    virtual auto SubchainLastIndexed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto SubchainMatchBlock(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const MatchingIndices& indices,
        const ReadView blockID,
        const VersionNumber version = DefaultIndexVersion) const noexcept
        -> bool = 0;
    virtual auto SubchainLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position = 0;
    virtual auto SubchainLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type) const noexcept -> block::Position = 0;
    virtual auto SubchainSetLastProcessed(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto SubchainSetLastScanned(
        const NodeID& balanceNode,
        const Subchain subchain,
        const FilterType type,
        const block::Position& position) const noexcept -> bool = 0;
    virtual auto TransactionLoadBitcoin(const ReadView txid) const noexcept
        -> std::unique_ptr<block::bitcoin::Transaction> = 0;

    virtual ~WalletDatabase() = default;
};
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::blockchain::client::internal
