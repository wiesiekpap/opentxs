// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/blockchain/sync/MessageType.hpp"

#ifndef OPENTXS_NETWORK_BLOCKCHAIN_SYNC_MESSAGE_HPP
#define OPENTXS_NETWORK_BLOCKCHAIN_SYNC_MESSAGE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/network/blockchain/sync/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
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
class Query;
class Request;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace sync
{
OPENTXS_EXPORT auto Factory(
    const api::Core& api,
    const zeromq::Message& in) noexcept -> std::unique_ptr<Base>;

class OPENTXS_EXPORT Base
{
public:
    struct Imp;

    auto asAcknowledgement() const noexcept -> const Acknowledgement&;
    auto asData() const noexcept -> const Data&;
    auto asQuery() const noexcept -> const Query&;
    auto asRequest() const noexcept -> const Request&;

    auto Serialize(zeromq::Message& out) const noexcept -> bool;
    auto Type() const noexcept -> MessageType;
    auto Version() const noexcept -> VersionNumber;

    Base() noexcept;

    virtual ~Base();

protected:
    std::unique_ptr<Imp> imp_;

    Base(std::unique_ptr<Imp> imp) noexcept;

private:
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;
};
}  // namespace sync
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
#endif
