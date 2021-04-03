// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_BAILMENTREPLY_HPP
#define OPENTXS_CORE_CONTRACT_PEER_BAILMENTREPLY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/peer/PeerReply.hpp"

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
class Bailment;
}  // namespace reply
}  // namespace peer
}  // namespace contract

using OTBailmentReply = SharedPimpl<contract::peer::reply::Bailment>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace reply
{
class Bailment : virtual public peer::Reply
{
public:
    OPENTXS_EXPORT ~Bailment() override = default;

protected:
    Bailment() noexcept = default;

private:
    friend OTBailmentReply;

#ifndef _WIN32
    OPENTXS_EXPORT Bailment* clone() const noexcept override = 0;
#endif

    Bailment(const Bailment&) = delete;
    Bailment(Bailment&&) = delete;
    Bailment& operator=(const Bailment&) = delete;
    Bailment& operator=(Bailment&&) = delete;
};
}  // namespace reply
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
