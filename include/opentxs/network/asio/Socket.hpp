// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/api/network/Network.hpp"

#ifndef OPENTXS_NETWORK_ASIO_SOCKET_HPP
#define OPENTXS_NETWORK_ASIO_SOCKET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <future>
#include <iosfwd>
#include <memory>

#include "opentxs/Bytes.hpp"
#include "opentxs/util/WorkType.hpp"

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
}  // namespace asio
}  // namespace network
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace asio
{
class Socket
{
public:
    struct Imp;

    using Asio = api::network::internal::Asio;
    using SendStatus = std::promise<bool>;
    using Notification = std::unique_ptr<SendStatus>;

    OPENTXS_EXPORT auto Close() noexcept -> void;
    OPENTXS_EXPORT auto Connect(const ReadView notify) noexcept -> bool;
    OPENTXS_EXPORT auto Receive(
        const ReadView notify,
        const OTZMQWorkType type,
        const std::size_t bytes) noexcept -> bool;
    OPENTXS_EXPORT auto Transmit(
        const ReadView data,
        Notification notifier) noexcept -> bool;

    Socket(const Endpoint& endpoint, Asio& asio) noexcept;
    OPENTXS_EXPORT Socket(Socket&&) noexcept;

    OPENTXS_EXPORT ~Socket();

private:
    Imp* imp_;

    Socket() noexcept = delete;
    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;
};
}  // namespace asio
}  // namespace network
}  // namespace opentxs
#endif
