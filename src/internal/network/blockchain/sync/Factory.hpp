// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/network/blockchain/sync/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace blockchain
{
namespace sync
{
class Acknowledgement;
class Base;
class Data;
class Request;
class Query;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
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
auto BlockchainSyncQuery() noexcept -> network::blockchain::sync::Query;
auto BlockchainSyncQuery(int) noexcept -> network::blockchain::sync::Query;
auto BlockchainSyncQuery_p(int) noexcept
    -> std::unique_ptr<network::blockchain::sync::Query>;
auto BlockchainSyncRequest() noexcept -> network::blockchain::sync::Request;
auto BlockchainSyncRequest(network::blockchain::sync::StateData in) noexcept
    -> network::blockchain::sync::Request;
auto BlockchainSyncRequest_p(network::blockchain::sync::StateData in) noexcept
    -> std::unique_ptr<network::blockchain::sync::Request>;
}  // namespace opentxs::factory
