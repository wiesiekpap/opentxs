// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_WALLET_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_WALLET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Account;
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Wallet
{
public:
    using const_iterator =
        opentxs::iterator::Bidirectional<const Wallet, const Account>;

    /// Throws std::out_of_range for invalid position
    virtual const_iterator::value_type& at(const std::size_t position) const
        noexcept(false) = 0;
    virtual const_iterator begin() const noexcept = 0;
    virtual const_iterator cbegin() const noexcept = 0;
    virtual const_iterator cend() const noexcept = 0;
    virtual opentxs::blockchain::Type Chain() const noexcept = 0;
    virtual const_iterator end() const noexcept = 0;
    virtual std::size_t size() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual ~Wallet() = default;

protected:
    Wallet() noexcept = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_BALANCELIST_HPP
