// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_ACCOUNT_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_ACCOUNT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/crypto/Subaccount.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Element;
class HD;
class Imported;
class PaymentCode;
class Wallet;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Identifier;
class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Account
{
public:
    struct OPENTXS_EXPORT HDAccounts {
        using value_type = HD;
        using const_iterator = opentxs::iterator::
            Bidirectional<const HDAccounts, const value_type>;

        /// Throws std::out_of_range for invalid position
        virtual const value_type& at(const std::size_t position) const
            noexcept(false) = 0;
        /// Throws std::out_of_range for invalid id
        virtual const value_type& at(const Identifier& id) const
            noexcept(false) = 0;
        virtual const_iterator begin() const noexcept = 0;
        virtual const_iterator cbegin() const noexcept = 0;
        virtual const_iterator cend() const noexcept = 0;
        virtual const_iterator end() const noexcept = 0;
        virtual std::size_t size() const noexcept = 0;

        OPENTXS_NO_EXPORT virtual ~HDAccounts() = default;
    };
    struct OPENTXS_EXPORT ImportedAccounts {
        using value_type = Imported;
        using const_iterator = opentxs::iterator::
            Bidirectional<const ImportedAccounts, const value_type>;

        /// Throws std::out_of_range for invalid position
        virtual const value_type& at(const std::size_t position) const
            noexcept(false) = 0;
        /// Throws std::out_of_range for invalid id
        virtual const value_type& at(const Identifier& id) const
            noexcept(false) = 0;
        virtual const_iterator begin() const noexcept = 0;
        virtual const_iterator cbegin() const noexcept = 0;
        virtual const_iterator cend() const noexcept = 0;
        virtual const_iterator end() const noexcept = 0;
        virtual std::size_t size() const noexcept = 0;

        OPENTXS_NO_EXPORT virtual ~ImportedAccounts() = default;
    };
    struct OPENTXS_EXPORT PaymentCodeAccounts {
        using value_type = PaymentCode;
        using const_iterator = opentxs::iterator::
            Bidirectional<const PaymentCodeAccounts, const value_type>;

        /// Throws std::out_of_range for invalid position
        virtual const value_type& at(const std::size_t position) const
            noexcept(false) = 0;
        /// Throws std::out_of_range for invalid id
        virtual const value_type& at(const Identifier& id) const
            noexcept(false) = 0;
        virtual const_iterator begin() const noexcept = 0;
        virtual const_iterator cbegin() const noexcept = 0;
        virtual const_iterator cend() const noexcept = 0;
        virtual const_iterator end() const noexcept = 0;
        virtual std::size_t size() const noexcept = 0;

        OPENTXS_NO_EXPORT virtual ~PaymentCodeAccounts() = default;
    };

    virtual const Identifier& AccountID() const noexcept = 0;
    virtual const HDAccounts& GetHD() const noexcept = 0;
    /// Throws std::out_of_range if no keys are available
    virtual const Element& GetNextChangeKey(const PasswordPrompt& reason) const
        noexcept(false) = 0;
    /// Throws std::out_of_range if no keys are available
    virtual const Element& GetNextDepositKey(const PasswordPrompt& reason) const
        noexcept(false) = 0;
    virtual std::string GetDepositAddress(
        const AddressStyle style,
        const PasswordPrompt& reason,
        const std::string& memo = "") const noexcept = 0;
    virtual std::string GetDepositAddress(
        const AddressStyle style,
        const Identifier& contact,
        const PasswordPrompt& reason,
        const std::string& memo = "") const noexcept = 0;
    virtual const ImportedAccounts& GetImported() const noexcept = 0;
    virtual const PaymentCodeAccounts& GetPaymentCode() const noexcept = 0;
    virtual const identifier::Nym& NymID() const noexcept = 0;
    virtual const Wallet& Parent() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual ~Account() = default;

protected:
    Account() noexcept = default;

private:
    Account(const Account&) = delete;
    Account(Account&&) = delete;
    Account& operator=(const Account&) = delete;
    Account& operator=(Account&&) = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_BALANCETREE_HPP
