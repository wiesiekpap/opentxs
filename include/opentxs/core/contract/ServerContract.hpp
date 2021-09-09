// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP
#define OPENTXS_CORE_CONTRACT_SERVERCONTRACT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/contract/Types.hpp"

namespace opentxs
{
namespace contract
{
class Server;
}  // namespace contract

namespace proto
{
class ServerContract;
}  // namespace proto

class PasswordPrompt;

using OTServerContract = SharedPimpl<contract::Server>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
class OPENTXS_EXPORT Server : virtual public opentxs::contract::Signable
{
public:
    using Endpoint = std::tuple<
        core::AddressType,
        contract::ProtocolVersion,
        std::string,     // hostname / address
        std::uint32_t,   // port
        VersionNumber>;  // version

    static const VersionNumber DefaultVersion;

    virtual auto ConnectInfo(
        std::string& strHostname,
        std::uint32_t& nPort,
        core::AddressType& actual,
        const core::AddressType& preferred) const -> bool = 0;
    virtual auto EffectiveName() const -> std::string = 0;
    using Signable::Serialize;
    virtual auto Serialize(AllocateOutput destination, bool includeNym = false)
        const -> bool = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(
        proto::ServerContract&,
        bool includeNym = false) const -> bool = 0;
    virtual auto Statistics(String& strContents) const -> bool = 0;
    virtual auto TransportKey() const -> const Data& = 0;
    virtual auto TransportKey(Data& pubkey, const PasswordPrompt& reason) const
        -> OTSecret = 0;

    virtual void InitAlias(const std::string& alias) = 0;

    ~Server() override = default;

protected:
    Server() noexcept = default;

private:
    friend OTServerContract;

#ifndef _WIN32
    auto clone() const noexcept -> Server* override = 0;
#endif

    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace contract
}  // namespace opentxs
#endif
