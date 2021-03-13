// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_IMPORTED_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_IMPORTED_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/api/client/blockchain/BalanceNode.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class Imported : virtual public BalanceNode
{
public:
    OPENTXS_EXPORT virtual ECKey Key() const = 0;

    OPENTXS_EXPORT ~Imported() override = default;

protected:
    Imported() noexcept = default;

private:
    Imported(const Imported&) = delete;
    Imported(Imported&&) = delete;
    Imported& operator=(const Imported&) = delete;
    Imported& operator=(Imported&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_IMPORTED_HPP
