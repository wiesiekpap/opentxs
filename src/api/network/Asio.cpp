// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/api/network/Asio.hpp"  // IWYU pragma: associated

#include <memory>

#include "api/network/Asio.hpp"
#include "opentxs/network/asio/Socket.hpp"

namespace opentxs::api::network
{
Asio::Asio(const opentxs::network::zeromq::Context& zmq) noexcept
    : imp_(std::make_unique<Imp>(zmq).release())
{
}

auto Asio::Init() noexcept -> void { imp_->Init(); }

auto Asio::MakeSocket(const opentxs::network::asio::Endpoint& endpoint)
    const noexcept -> opentxs::network::asio::Socket
{
    return {endpoint, *const_cast<Asio*>(this)->imp_};
}

auto Asio::NotificationEndpoint() const noexcept -> const char*
{
    return imp_->Endpoint();
}

auto Asio::Resolve(std::string_view server, std::uint16_t port) const noexcept
    -> Resolved
{
    return imp_->Resolve(server, port);
}

auto Asio::Shutdown() noexcept -> void { imp_->Shutdown(); }

Asio::~Asio() { std::unique_ptr<Imp>{imp_}.reset(); }
}  // namespace opentxs::api::network
