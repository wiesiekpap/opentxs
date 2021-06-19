// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_SUBACCOUNT_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_SUBACCOUNT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Account;
class Element;
}  // namespace crypto
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Subaccount
{
public:
    using Txid = opentxs::blockchain::block::Txid;

    /// Throws std::out_of_range for invalid index
    virtual const crypto::Element& BalanceElement(
        const Subchain type,
        const Bip32Index index) const noexcept(false) = 0;
    virtual const Identifier& ID() const noexcept = 0;
    virtual const Account& Parent() const noexcept = 0;
    virtual SubaccountType Type() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual ~Subaccount() = default;

protected:
    Subaccount() noexcept = default;

private:
    Subaccount(const Subaccount&) = delete;
    Subaccount(Subaccount&&) = delete;
    Subaccount& operator=(const Subaccount&) = delete;
    Subaccount& operator=(Subaccount&&) = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_BALANCENODE_HPP
