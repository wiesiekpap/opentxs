// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_HD_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_HD_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/api/client/blockchain/Deterministic.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class HD : virtual public Deterministic
{
public:
    OPENTXS_EXPORT ~HD() override = default;

protected:
    HD() noexcept = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};

}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_HDCHAIN_HPP
