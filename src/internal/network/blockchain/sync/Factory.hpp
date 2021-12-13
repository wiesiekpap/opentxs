// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU praga: no_include "opentxs/core/contract/ContractType.hpp"

#pragma once

#include <memory>

#include "opentxs/core/contract/Types.hpp"
#include "opentxs/network/blockchain/sync/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace contract
{
class Server;
class Unit;
}  // namespace contract

namespace network
{
namespace blockchain
{
namespace sync
{
class Acknowledgement;
class Base;
class Data;
class PublishContract;
class PublishContractReply;
class Query;
class QueryContract;
class QueryContractReply;
class Request;
}  // namespace sync
}  // namespace blockchain

namespace identity
{
class Nym;
}  // namespace identity

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
}  // namespace opentxs

namespace opentxs::factory
{
auto BlockchainSyncAcknowledgement() noexcept
    -> network::blockchain::sync::Acknowledgement;
auto BlockchainSyncAcknowledgement(
    network::blockchain::sync::StateData in,
    std::string endpoint) noexcept
    -> network::blockchain::sync::Acknowledgement;
auto BlockchainSyncAcknowledgement_p(
    network::blockchain::sync::StateData in,
    std::string endpoint) noexcept
    -> std::unique_ptr<network::blockchain::sync::Acknowledgement>;
auto BlockchainSyncData() noexcept -> network::blockchain::sync::Data;
auto BlockchainSyncData(
    WorkType type,
    network::blockchain::sync::State state,
    network::blockchain::sync::SyncData blocks,
    ReadView cfheader) noexcept -> network::blockchain::sync::Data;
auto BlockchainSyncData_p(
    WorkType type,
    network::blockchain::sync::State state,
    network::blockchain::sync::SyncData blocks,
    ReadView cfheader) noexcept
    -> std::unique_ptr<network::blockchain::sync::Data>;
auto BlockchainSyncMessage(
    const api::Session& api,
    const network::zeromq::Message& in) noexcept
    -> std::unique_ptr<network::blockchain::sync::Base>;
auto BlockchainSyncPublishContract() noexcept
    -> network::blockchain::sync::PublishContract;
auto BlockchainSyncPublishContract(const identity::Nym& payload) noexcept
    -> network::blockchain::sync::PublishContract;
auto BlockchainSyncPublishContract(const contract::Server& payload) noexcept
    -> network::blockchain::sync::PublishContract;
auto BlockchainSyncPublishContract(const contract::Unit& payload) noexcept
    -> network::blockchain::sync::PublishContract;
auto BlockchainSyncPublishContract_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::blockchain::sync::PublishContract>;
auto BlockchainSyncPublishContractReply() noexcept
    -> network::blockchain::sync::PublishContractReply;
auto BlockchainSyncPublishContractReply(
    const Identifier& id,
    const bool success) noexcept
    -> network::blockchain::sync::PublishContractReply;
auto BlockchainSyncPublishContractReply_p(
    const api::Session& api,
    const ReadView id,
    const ReadView success) noexcept
    -> std::unique_ptr<network::blockchain::sync::PublishContractReply>;
auto BlockchainSyncQuery() noexcept -> network::blockchain::sync::Query;
auto BlockchainSyncQuery(int) noexcept -> network::blockchain::sync::Query;
auto BlockchainSyncQuery_p(int) noexcept
    -> std::unique_ptr<network::blockchain::sync::Query>;
auto BlockchainSyncQueryContract() noexcept
    -> network::blockchain::sync::QueryContract;
auto BlockchainSyncQueryContract(const Identifier& id) noexcept
    -> network::blockchain::sync::QueryContract;
auto BlockchainSyncQueryContract_p(
    const api::Session& api,
    const ReadView id) noexcept
    -> std::unique_ptr<network::blockchain::sync::QueryContract>;
auto BlockchainSyncQueryContractReply() noexcept
    -> network::blockchain::sync::QueryContractReply;
auto BlockchainSyncQueryContractReply(const Identifier& id) noexcept
    -> network::blockchain::sync::QueryContractReply;
auto BlockchainSyncQueryContractReply(const identity::Nym& payload) noexcept
    -> network::blockchain::sync::QueryContractReply;
auto BlockchainSyncQueryContractReply(const contract::Server& payload) noexcept
    -> network::blockchain::sync::QueryContractReply;
auto BlockchainSyncQueryContractReply(const contract::Unit& payload) noexcept
    -> network::blockchain::sync::QueryContractReply;
auto BlockchainSyncQueryContractReply_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::blockchain::sync::QueryContractReply>;
auto BlockchainSyncRequest() noexcept -> network::blockchain::sync::Request;
auto BlockchainSyncRequest(network::blockchain::sync::StateData in) noexcept
    -> network::blockchain::sync::Request;
auto BlockchainSyncRequest_p(network::blockchain::sync::StateData in) noexcept
    -> std::unique_ptr<network::blockchain::sync::Request>;
}  // namespace opentxs::factory
