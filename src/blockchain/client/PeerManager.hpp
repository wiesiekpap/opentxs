// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/container/flat_set.hpp>
#include <atomic>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Forward.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin

class Block;
}  // namespace block

namespace p2p
{
namespace internal
{
struct Address;
struct Peer;
}  // namespace internal

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Sender;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

class Flag;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::client::implementation
{
class PeerManager final : virtual public internal::PeerManager,
                          public Worker<PeerManager, api::Core>
{
public:
    class IncomingConnectionManager;

    struct Peers {
        using Endpoint = std::unique_ptr<p2p::internal::Address>;

        const Type chain_;

        auto Count() const noexcept -> std::size_t { return count_.load(); }

        auto AddIncoming(const int id, Endpoint endpoint) noexcept -> void
        {
            add_peer(id, std::move(endpoint));
        }
        auto AddListener(
            const p2p::Address& address,
            std::promise<bool>& promise) noexcept -> void;
        auto AddPeer(
            const p2p::Address& address,
            std::promise<bool>& promise) noexcept -> void;
        auto ConstructPeer(Endpoint endpoint) noexcept -> int;
        auto Disconnect(const int id) noexcept -> void;
        auto Run() noexcept -> bool;
        auto Shutdown() noexcept -> void;

        Peers(
            const api::Core& api,
            const internal::Config& config,
            const internal::Network& network,
            const internal::HeaderOracle& headers,
            const internal::FilterOracle& filter,
            const internal::BlockOracle& block,
            const internal::PeerDatabase& database,
            const internal::PeerManager& parent,
            const api::client::blockchain::BlockStorage policy,
            const Flag& running,
            const std::string& shutdown,
            const Type chain,
            const std::string& seednode,
            const blockchain::client::internal::IO& context,
            const std::size_t peerTarget) noexcept;

        ~Peers();

    private:
        using Peer = std::unique_ptr<p2p::internal::Peer>;
        using Resolver = boost::asio::ip::tcp::resolver;
        using Addresses = boost::container::flat_set<OTIdentifier>;

        const api::Core& api_;
        const internal::Config& config_;
        const internal::Network& network_;
        const internal::HeaderOracle& headers_;
        const internal::FilterOracle& filter_;
        const internal::BlockOracle& block_;
        const internal::PeerDatabase& database_;
        const internal::PeerManager& parent_;
        const api::client::blockchain::BlockStorage policy_;
        const blockchain::client::internal::IO& context_;
        const Flag& running_;
        const std::string& shutdown_endpoint_;
        const bool invalid_peer_;
        const OTData localhost_peer_;
        const OTData default_peer_;
        const std::set<p2p::Service> preferred_services_;
        mutable Resolver resolver_;
        std::atomic<int> next_id_;
        std::atomic<std::size_t> minimum_peers_;
        std::map<int, Peer> peers_;
        std::map<OTIdentifier, int> active_;
        std::atomic<std::size_t> count_;
        Addresses connected_;
        std::unique_ptr<IncomingConnectionManager> incoming_zmq_;

        static auto get_preferred_services(
            const internal::Config& config) noexcept -> std::set<p2p::Service>;
        static auto set_default_peer(
            const std::string node,
            const Data& localhost,
            bool& invalidPeer) noexcept -> OTData;

        auto get_default_peer() const noexcept -> Endpoint;
        auto get_dns_peer() const noexcept -> Endpoint;
        auto get_fallback_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto get_peer() const noexcept -> Endpoint;
        auto get_preferred_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto is_not_connected(const p2p::Address& endpoint) const noexcept
            -> bool;

        auto add_peer(Endpoint endpoint) noexcept -> int;
        auto add_peer(const int id, Endpoint endpoint) noexcept -> int;
        auto peer_factory(Endpoint endpoint, const int id) noexcept
            -> std::unique_ptr<p2p::internal::Peer>;
    };

    enum class Work : OTZMQWorkType {
        Disconnect = OT_ZMQ_INTERNAL_SIGNAL + 0,
        AddPeer = OT_ZMQ_INTERNAL_SIGNAL + 1,
        AddListener = OT_ZMQ_INTERNAL_SIGNAL + 2,
        IncomingPeer = OT_ZMQ_INTERNAL_SIGNAL + 3,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = value(WorkType::Shutdown),
    };

    auto AddIncomingPeer(const int id, std::uintptr_t endpoint) const noexcept
        -> void final;
    auto AddPeer(const p2p::Address& address) const noexcept -> bool final;
    auto BroadcastBlock(const block::Block& block) const noexcept -> bool final;
    auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool final;
    auto Database() const noexcept -> const internal::PeerDatabase& final
    {
        return database_;
    }
    auto Connect() noexcept -> bool final;
    auto Disconnect(const int id) const noexcept -> void final;
    auto Endpoint(const Task type) const noexcept -> std::string final
    {
        return jobs_.Endpoint(type);
    }
    auto GetPeerCount() const noexcept -> std::size_t final
    {
        return peers_.Count();
    }
    auto Heartbeat() const noexcept -> void final
    {
        jobs_.Dispatch(Task::Heartbeat);
    }
    auto JobReady(const Task type) const noexcept -> void final;
    auto Listen(const p2p::Address& address) const noexcept -> bool final;
    auto RequestBlock(const block::Hash& block) const noexcept -> bool final;
    auto RequestBlocks(const std::vector<ReadView>& hashes) const noexcept
        -> bool final;
    auto RequestHeaders() const noexcept -> bool final;
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return stop_worker();
    }

    auto init() noexcept -> void final;

    PeerManager(
        const api::Core& api,
        const internal::Config& config,
        const internal::Network& network,
        const internal::HeaderOracle& headers,
        const internal::FilterOracle& filter,
        const internal::BlockOracle& block,
        const internal::PeerDatabase& database,
        const blockchain::client::internal::IO& io,
        const Type chain,
        const api::client::blockchain::BlockStorage policy,
        const std::string& seednode,
        const std::string& shutdown) noexcept;

    ~PeerManager() final;

private:
    friend Worker<PeerManager, api::Core>;

    struct Jobs {
        auto Endpoint(const Task type) const noexcept -> std::string;
        auto Work(const Task task, std::promise<void>* promise = nullptr)
            const noexcept -> OTZMQMessage;

        auto Dispatch(const Task type) noexcept -> void;
        auto Dispatch(zmq::Message& work) noexcept -> void;
        auto Shutdown() noexcept -> void;

        Jobs(const api::Core& api) noexcept;

    private:
        using EndpointMap = std::map<Task, std::string>;
        using SocketMap = std::map<Task, zmq::socket::Sender*>;

        const zmq::Context& zmq_;
        OTZMQPushSocket getheaders_;
        OTZMQPublishSocket getcfheaders_;
        OTZMQPublishSocket getcfilters_;
        OTZMQPublishSocket getblocks_;
        OTZMQPublishSocket heartbeat_;
        OTZMQPushSocket getblock_;
        OTZMQPushSocket broadcast_transaction_;
        OTZMQPublishSocket broadcast_block_;
        const EndpointMap endpoint_map_;
        const SocketMap socket_map_;

        static auto listen(
            EndpointMap& map,
            const Task type,
            const zmq::socket::Sender& socket) noexcept -> void;

        Jobs() = delete;
    };

    const internal::PeerDatabase& database_;
    const internal::IO& io_context_;
    mutable Jobs jobs_;
    mutable Peers peers_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    static auto peer_target(
        const Type chain,
        const api::client::blockchain::BlockStorage policy) noexcept
        -> std::size_t;

    auto pipeline(zmq::Message& message) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    PeerManager() = delete;
    PeerManager(const PeerManager&) = delete;
    PeerManager(PeerManager&&) = delete;
    auto operator=(const PeerManager&) -> PeerManager& = delete;
    auto operator=(PeerManager&&) -> PeerManager& = delete;
};
}  // namespace opentxs::blockchain::client::implementation
