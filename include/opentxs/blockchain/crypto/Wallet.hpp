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
namespace api
{
namespace client
{
class Blockchain;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace crypto
{
namespace internal
{
struct Wallet;
}  // namespace internal

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
        opentxs::iterator::Bidirectional<const Wallet, const crypto::Account>;

    /// Throws std::out_of_range for invalid position
    virtual auto at(const std::size_t position) const noexcept(false)
        -> const_iterator::value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto Chain() const noexcept -> opentxs::blockchain::Type = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    virtual auto Account(const identifier::Nym& id) const noexcept
        -> Account& = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> internal::Wallet& = 0;
    virtual auto Parent() const noexcept -> const api::client::Blockchain& = 0;

    OPENTXS_NO_EXPORT virtual ~Wallet() = default;

protected:
    Wallet() noexcept = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_BALANCELIST_HPP
