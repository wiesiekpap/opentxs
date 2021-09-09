// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "core/contract/peer/PeerReply.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
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
class Outbailment final : public reply::Outbailment,
                          public peer::implementation::Reply
{
public:
    Outbailment(
        const api::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms);
    Outbailment(
        const api::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);

    ~Outbailment() final = default;

    auto asOutbailment() const noexcept -> const reply::Outbailment& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    auto clone() const noexcept -> Outbailment* final
    {
        return new Outbailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    Outbailment() = delete;
    Outbailment(const Outbailment&);
    Outbailment(Outbailment&&) = delete;
    auto operator=(const Outbailment&) -> Outbailment& = delete;
    auto operator=(Outbailment&&) -> Outbailment& = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
