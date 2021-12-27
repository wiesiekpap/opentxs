// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU praga: no_include "opentxs/core/contract/ContractType.hpp"

#pragma once

#include <memory>

#include "opentxs/core/contract/Types.hpp"
#include "opentxs/network/p2p/Types.hpp"
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

namespace identity
{
class Nym;
}  // namespace identity

namespace network
{
namespace p2p
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
}  // namespace p2p

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
}  // namespace opentxs

namespace opentxs::factory
{
auto BlockchainSyncAcknowledgement() noexcept -> network::p2p::Acknowledgement;
auto BlockchainSyncAcknowledgement(
    network::p2p::StateData in,
    std::string endpoint) noexcept -> network::p2p::Acknowledgement;
auto BlockchainSyncAcknowledgement_p(
    network::p2p::StateData in,
    std::string endpoint) noexcept
    -> std::unique_ptr<network::p2p::Acknowledgement>;
auto BlockchainSyncData() noexcept -> network::p2p::Data;
auto BlockchainSyncData(
    WorkType type,
    network::p2p::State state,
    network::p2p::SyncData blocks,
    ReadView cfheader) noexcept -> network::p2p::Data;
auto BlockchainSyncData_p(
    WorkType type,
    network::p2p::State state,
    network::p2p::SyncData blocks,
    ReadView cfheader) noexcept -> std::unique_ptr<network::p2p::Data>;
auto BlockchainSyncMessage(
    const api::Session& api,
    const network::zeromq::Message& in) noexcept
    -> std::unique_ptr<network::p2p::Base>;
auto BlockchainSyncPublishContract() noexcept -> network::p2p::PublishContract;
auto BlockchainSyncPublishContract(const identity::Nym& payload) noexcept
    -> network::p2p::PublishContract;
auto BlockchainSyncPublishContract(const contract::Server& payload) noexcept
    -> network::p2p::PublishContract;
auto BlockchainSyncPublishContract(const contract::Unit& payload) noexcept
    -> network::p2p::PublishContract;
auto BlockchainSyncPublishContract_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::p2p::PublishContract>;
auto BlockchainSyncPublishContractReply() noexcept
    -> network::p2p::PublishContractReply;
auto BlockchainSyncPublishContractReply(
    const Identifier& id,
    const bool success) noexcept -> network::p2p::PublishContractReply;
auto BlockchainSyncPublishContractReply_p(
    const api::Session& api,
    const ReadView id,
    const ReadView success) noexcept
    -> std::unique_ptr<network::p2p::PublishContractReply>;
auto BlockchainSyncQuery() noexcept -> network::p2p::Query;
auto BlockchainSyncQuery(int) noexcept -> network::p2p::Query;
auto BlockchainSyncQuery_p(int) noexcept
    -> std::unique_ptr<network::p2p::Query>;
auto BlockchainSyncQueryContract() noexcept -> network::p2p::QueryContract;
auto BlockchainSyncQueryContract(const Identifier& id) noexcept
    -> network::p2p::QueryContract;
auto BlockchainSyncQueryContract_p(
    const api::Session& api,
    const ReadView id) noexcept -> std::unique_ptr<network::p2p::QueryContract>;
auto BlockchainSyncQueryContractReply() noexcept
    -> network::p2p::QueryContractReply;
auto BlockchainSyncQueryContractReply(const Identifier& id) noexcept
    -> network::p2p::QueryContractReply;
auto BlockchainSyncQueryContractReply(const identity::Nym& payload) noexcept
    -> network::p2p::QueryContractReply;
auto BlockchainSyncQueryContractReply(const contract::Server& payload) noexcept
    -> network::p2p::QueryContractReply;
auto BlockchainSyncQueryContractReply(const contract::Unit& payload) noexcept
    -> network::p2p::QueryContractReply;
auto BlockchainSyncQueryContractReply_p(
    const api::Session& api,
    const contract::Type type,
    const ReadView id,
    const ReadView payload) noexcept
    -> std::unique_ptr<network::p2p::QueryContractReply>;
auto BlockchainSyncRequest() noexcept -> network::p2p::Request;
auto BlockchainSyncRequest(network::p2p::StateData in) noexcept
    -> network::p2p::Request;
auto BlockchainSyncRequest_p(network::p2p::StateData in) noexcept
    -> std::unique_ptr<network::p2p::Request>;
}  // namespace opentxs::factory
