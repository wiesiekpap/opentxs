// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/blockchain/sync/MessageType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/network/blockchain/sync/Types.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
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

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::blockchain::sync
{
class OPENTXS_EXPORT Base
{
public:
    class Imp;

    auto asAcknowledgement() const noexcept -> const Acknowledgement&;
    auto asData() const noexcept -> const Data&;
    auto asPublishContract() const noexcept -> const PublishContract&;
    auto asPublishContractReply() const noexcept -> const PublishContractReply&;
    auto asQuery() const noexcept -> const Query&;
    auto asQueryContract() const noexcept -> const QueryContract&;
    auto asQueryContractReply() const noexcept -> const QueryContractReply&;
    auto asRequest() const noexcept -> const Request&;

    auto Serialize(zeromq::Message& out) const noexcept -> bool;
    auto Type() const noexcept -> MessageType;
    auto Version() const noexcept -> VersionNumber;

    Base() noexcept;

    virtual ~Base();

protected:
    Imp* imp_;

    Base(Imp* imp) noexcept;

private:
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;
};
}  // namespace opentxs::network::blockchain::sync
