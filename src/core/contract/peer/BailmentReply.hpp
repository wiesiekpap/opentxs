// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "core/contract/peer/PeerReply.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
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
class Bailment final : public reply::Bailment,
                       public peer::implementation::Reply
{
public:
    Bailment(
        const api::Core& api,
        const Nym_p& nym,
        const SerializedType& serialized);
    Bailment(
        const api::Core& api,
        const Nym_p& nym,
        const identifier::Nym& initiator,
        const Identifier& request,
        const identifier::Server& server,
        const std::string& terms);

    ~Bailment() final = default;

    auto asBailment() const noexcept -> const reply::Bailment& final
    {
        return *this;
    }

private:
    friend opentxs::Factory;

    auto clone() const noexcept -> Bailment* final
    {
        return new Bailment(*this);
    }
    auto IDVersion(const Lock& lock) const -> SerializedType final;

    Bailment() = delete;
    Bailment(const Bailment&);
    Bailment(Bailment&&) = delete;
    auto operator=(const Bailment&) -> Bailment& = delete;
    auto operator=(Bailment&&) -> Bailment& = delete;
};
}  // namespace opentxs::contract::peer::reply::implementation
