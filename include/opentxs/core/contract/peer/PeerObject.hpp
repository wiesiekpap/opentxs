// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP
#define OPENTXS_CORE_CONTRACT_PEER_PEEROBJECT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{
namespace blind
{
class Purse;
}  // namespace blind

namespace proto
{
class PeerObject;
}  // namespace proto

class PeerObject;

using OTPeerObject = Pimpl<PeerObject>;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT PeerObject
{
public:
    virtual auto Message() const -> const std::unique_ptr<std::string>& = 0;
    virtual auto Nym() const -> const Nym_p& = 0;
    virtual auto Payment() const -> const std::unique_ptr<std::string>& = 0;
#if OT_CASH
    virtual auto Purse() const -> std::shared_ptr<blind::Purse> = 0;
#endif
    virtual auto Request() const -> const OTPeerRequest = 0;
    virtual auto Reply() const -> const OTPeerReply = 0;
    OPENTXS_NO_EXPORT virtual auto Serialize(proto::PeerObject&) const
        -> bool = 0;
    virtual auto Type() const -> contract::peer::PeerObjectType = 0;
    virtual auto Validate() const -> bool = 0;

    virtual auto Message() -> std::unique_ptr<std::string>& = 0;
    virtual auto Payment() -> std::unique_ptr<std::string>& = 0;
#if OT_CASH
    virtual auto Purse() -> std::shared_ptr<blind::Purse>& = 0;
#endif

    virtual ~PeerObject() = default;

protected:
    PeerObject() = default;

private:
    PeerObject(const PeerObject&) = delete;
    PeerObject(PeerObject&&) = delete;
    auto operator=(const PeerObject&) -> PeerObject& = delete;
    auto operator=(PeerObject&&) -> PeerObject& = delete;
};
}  // namespace opentxs
#endif
