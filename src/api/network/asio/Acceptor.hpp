// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <functional>

#include "opentxs/api/network/Asio.hpp"

namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio
}  // namespace boost

namespace opentxs
{
namespace api
{
namespace network
{
namespace internal
{
struct Asio;
}  // namespace internal
}  // namespace network
}  // namespace api

namespace network
{
namespace asio
{
class Endpoint;
class Socket;
}  // namespace asio
}  // namespace network
}  // namespace opentxs

namespace opentxs::api::network::asio
{
class Acceptor
{
public:
    using Callback = Asio::AcceptCallback;

    auto Start() noexcept -> void;
    auto Stop() noexcept -> void;

    Acceptor(
        const opentxs::network::asio::Endpoint& endpoint,
        internal::Asio& asio,
        boost::asio::io_context& ios,
        Callback&& cb) noexcept(false);

    ~Acceptor();

private:
    struct Imp;

    Imp* imp_;

    Acceptor() = delete;
    Acceptor(const Acceptor&) = delete;
    Acceptor(Acceptor&&) = delete;
    Acceptor& operator=(const Acceptor&) = delete;
    Acceptor& operator=(Acceptor&&) = delete;
};
}  // namespace opentxs::api::network::asio
