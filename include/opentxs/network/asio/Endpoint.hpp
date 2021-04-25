// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ASIO_ENDPOINT_HPP
#define OPENTXS_NETWORK_ASIO_ENDPOINT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/Bytes.hpp"

namespace opentxs
{
namespace network
{
namespace asio
{
class Endpoint
{
public:
    struct Imp;

    using Port = std::uint16_t;
    enum class Type : std::int8_t {
        none = 0,
        ipv4 = 1,
        ipv6 = 2,
    };

    OPENTXS_EXPORT auto GetAddress() const noexcept -> std::string;
    OPENTXS_EXPORT auto GetBytes() const noexcept -> ReadView;
    auto GetInternal() const noexcept -> const Imp&;
    OPENTXS_EXPORT auto GetMapped() const noexcept -> std::string;
    OPENTXS_EXPORT auto GetPort() const noexcept -> Port;
    OPENTXS_EXPORT auto GetType() const noexcept -> Type;
    OPENTXS_EXPORT auto str() const noexcept -> std::string;

    /// throws std::runtime_error for bad params
    OPENTXS_EXPORT Endpoint(Type type, ReadView bytes, Port port) noexcept(
        false);
    OPENTXS_EXPORT Endpoint() noexcept;
    OPENTXS_EXPORT Endpoint(const Endpoint&) noexcept;
    OPENTXS_EXPORT Endpoint(Endpoint&&) noexcept;

    OPENTXS_EXPORT auto operator=(const Endpoint&) noexcept -> Endpoint&;
    OPENTXS_EXPORT auto operator=(Endpoint&&) noexcept -> Endpoint&;

    OPENTXS_EXPORT ~Endpoint();

private:
    Imp* imp_;
};
}  // namespace asio
}  // namespace network
}  // namespace opentxs
#endif
