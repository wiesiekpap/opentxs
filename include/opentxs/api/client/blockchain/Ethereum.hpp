// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_ETHEREUM_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_ETHEREUM_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/api/client/blockchain/Imported.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class Ethereum : virtual public Imported
{
public:
    using Amount = opentxs::blockchain::Amount;
    using Nonce = Amount;

    OPENTXS_EXPORT virtual Amount GetBalance() const noexcept = 0;
    OPENTXS_EXPORT virtual Nonce GetNonce() const noexcept = 0;
    OPENTXS_EXPORT virtual Nonce IncrementNonce() const noexcept = 0;
    OPENTXS_EXPORT virtual void SetBalance(
        const Amount balance) const noexcept = 0;
    OPENTXS_EXPORT virtual void SetNonce(const Nonce nonce) const noexcept = 0;

    OPENTXS_EXPORT ~Ethereum() override = default;

protected:
    Ethereum() noexcept = default;

private:
    Ethereum(const Ethereum&) = delete;
    Ethereum(Ethereum&&) = delete;
    Ethereum& operator=(const Ethereum&) = delete;
    Ethereum& operator=(Ethereum&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_ETHEREUM_HPP
