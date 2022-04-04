// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs{ class PasswordPrompt; }
namespace opentxs::api{ class Session; }
namespace opentxs::api::network{ class ZMQ; }
namespace opentxs::network{ class ServerConnection; }
namespace opentxs::network::zeromq::socket { class Publish; }
namespace opentxs::otx::context { class Server; }

namespace opentxs::network
{
class OPENTXS_EXPORT ServerConnection
{
public:
    class Imp;

    enum class Push : bool {
        Enable = true,
        Disable = false,
    };

    static auto Factory(
        const api::Session& api,
        const api::network::ZMQ& zmq,
        const zeromq::socket::Publish& updates,
        const OTServerContract& contract) -> ServerConnection;

    auto ChangeAddressType(const AddressType type) -> bool;
    auto ClearProxy() -> bool;
    auto EnableProxy() -> bool;
    auto Send(
        const otx::context::Server& context,
        const Message& message,
        const PasswordPrompt& reason,
        const Push push = Push::Enable) -> otx::client::NetworkReplyMessage;
    auto Status() const -> bool;

    virtual auto swap(ServerConnection& rhs) noexcept -> void;

    OPENTXS_NO_EXPORT ServerConnection(Imp* imp) noexcept;
    ServerConnection(const ServerConnection&) noexcept = delete;
    ServerConnection(ServerConnection&&) noexcept;
    auto operator=(const ServerConnection&) noexcept -> ServerConnection& = delete;
    auto operator=(ServerConnection&&) noexcept -> ServerConnection&;

    virtual ~ServerConnection();

private:
    Imp* imp_;
};
}  // namespace opentxs::network
