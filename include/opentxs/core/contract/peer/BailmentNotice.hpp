// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP
#define OPENTXS_CORE_CONTRACT_PEER_BAILMENTNOTICE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/SharedPimpl.hpp"
#include "opentxs/core/contract/peer/PeerRequest.hpp"

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class BailmentNotice;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTBailmentNotice = SharedPimpl<contract::peer::request::BailmentNotice>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class OPENTXS_EXPORT BailmentNotice : virtual public peer::Request
{
public:
    ~BailmentNotice() override = default;

protected:
    BailmentNotice() noexcept = default;

private:
    friend OTBailmentNotice;

#ifndef _WIN32
    auto clone() const noexcept -> BailmentNotice* override = 0;
#endif

    BailmentNotice(const BailmentNotice&) = delete;
    BailmentNotice(BailmentNotice&&) = delete;
    auto operator=(const BailmentNotice&) -> BailmentNotice& = delete;
    auto operator=(BailmentNotice&&) -> BailmentNotice& = delete;
};
}  // namespace request
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
