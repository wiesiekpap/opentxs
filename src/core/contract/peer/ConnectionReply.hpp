// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "core/contract/peer/PeerReply.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace identifier
{
class Nym;
class Server;
}  // namespace identifier

class Factory;
class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::contract::peer::reply::implementation
{
class Connection final : public reply::Connection,
                         public peer::implementation::Reply
{
public:
    Connection(
        const api::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Connection(
        const api::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const bool ack,
        const std::string& url,
        const std::string& login,
        const std::string& password,
        const std::string& key);

    ~Connection() final = default;

    auto asConnection() const noexcept -> const reply::Connection& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    const bool success_;
    const std::string url_;
    const std::string login_;
    const std::string password_;
    const std::string key_;

    auto clone() const noexcept -> Connection* final
    {
        return new Connection(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    Connection() = delete;
    Connection(const Connection&);
    Connection(Connection&&) = delete;
    auto operator=(const Connection&) -> Connection& = delete;
    auto operator=(Connection&&) -> Connection& = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
