// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_CONTRACT_PEER_STORESECRET_HPP
#define OPENTXS_CORE_CONTRACT_PEER_STORESECRET_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

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
class StoreSecret;
}  // namespace request
}  // namespace peer
}  // namespace contract

using OTStoreSecret = SharedPimpl<contract::peer::request::StoreSecret>;
}  // namespace opentxs

namespace opentxs
{
namespace contract
{
namespace peer
{
namespace request
{
class StoreSecret : virtual public peer::Request
{
public:
    OPENTXS_EXPORT ~StoreSecret() override = default;

protected:
    StoreSecret() noexcept = default;

private:
    friend OTStoreSecret;

#ifndef _WIN32
    OPENTXS_EXPORT StoreSecret* clone() const noexcept override = 0;
#endif

    StoreSecret(const StoreSecret&) = delete;
    StoreSecret(StoreSecret&&) = delete;
    StoreSecret& operator=(const StoreSecret&) = delete;
    StoreSecret& operator=(StoreSecret&&) = delete;
};
}  // namespace request
}  // namespace peer
}  // namespace contract
}  // namespace opentxs
#endif
