// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CLIENT_OTCLIENT_HPP
#define OPENTXS_CLIENT_OTCLIENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/otx/consensus/Server.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

namespace contract
{
class Unit;
}  // namespace contract

namespace otx
{
namespace context
{
class Server;
}  // namespace context
}  // namespace otx

class Account;
class Identifier;
class Message;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
class OTClient
{
public:
    auto ProcessUserCommand(
        const MessageType requestedCommand,
        otx::context::Server& context,
        Message& theMessage,
        const Identifier& pHisNymID,
        const Identifier& pHisAcctID,
        const PasswordPrompt& reason,
        const Amount lTransactionAmount = 0,
        const Account* pAccount = nullptr,
        const contract::Unit* pMyUnitDefinition = nullptr) -> std::int32_t;

    explicit OTClient(const api::Core& api);

protected:
    const api::Core& api_;
};
}  // namespace opentxs
#endif
