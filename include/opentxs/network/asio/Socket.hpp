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
class OPENTXS_EXPORT Socket
{
public:
    struct Imp;

    using Asio = api::network::internal::Asio;
    using SendStatus = std::promise<bool>;
    using Notification = std::unique_ptr<SendStatus>;

    auto Close() noexcept -> void;
    auto Connect(const ReadView notify) noexcept -> bool;
    auto Receive(
        const ReadView notify,
        const OTZMQWorkType type,
        const std::size_t bytes) noexcept -> bool;
    auto Transmit(const ReadView data, Notification notifier) noexcept -> bool;

    OPENTXS_NO_EXPORT Socket(Imp* imp) noexcept;
    Socket(Socket&&) noexcept;

    ~Socket();

private:
    Imp* imp_;

    Socket() noexcept = delete;
    Socket(const Socket&) = delete;
    auto operator=(const Socket&) -> Socket& = delete;
    auto operator=(Socket&&) -> Socket& = delete;
};
}  // namespace asio
}  // namespace network
}  // namespace opentxs
#endif
