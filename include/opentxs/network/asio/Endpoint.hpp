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
class OPENTXS_EXPORT Endpoint
{
public:
    struct Imp;

    using Port = std::uint16_t;
    enum class Type : std::int8_t {
        none = 0,
        ipv4 = 1,
        ipv6 = 2,
    };

    auto GetAddress() const noexcept -> std::string;
    auto GetBytes() const noexcept -> ReadView;
    OPENTXS_NO_EXPORT auto GetInternal() const noexcept -> const Imp&;
    auto GetMapped() const noexcept -> std::string;
    auto GetPort() const noexcept -> Port;
    auto GetType() const noexcept -> Type;
    auto str() const noexcept -> std::string;

    /// throws std::runtime_error for bad params
    Endpoint(Type type, ReadView bytes, Port port) noexcept(false);
    Endpoint() noexcept;
    Endpoint(const Endpoint&) noexcept;
    Endpoint(Endpoint&&) noexcept;

    auto operator=(const Endpoint&) noexcept -> Endpoint&;
    auto operator=(Endpoint&&) noexcept -> Endpoint&;

    ~Endpoint();

private:
    Imp* imp_;
};
}  // namespace asio
}  // namespace network
}  // namespace opentxs
#endif
