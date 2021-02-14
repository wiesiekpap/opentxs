// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCETREE_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCETREE_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class BalanceTree
{
public:
    struct HDAccounts {
        using value_type = HD;
        using const_iterator = opentxs::iterator::
            Bidirectional<const HDAccounts, const value_type>;

        /// Throws std::out_of_range for invalid position
        OPENTXS_EXPORT virtual const value_type& at(
            const std::size_t position) const noexcept(false) = 0;
        /// Throws std::out_of_range for invalid id
        OPENTXS_EXPORT virtual const value_type& at(const Identifier& id) const
            noexcept(false) = 0;
        OPENTXS_EXPORT virtual const_iterator begin() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator cbegin() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator cend() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator end() const noexcept = 0;
        OPENTXS_EXPORT virtual std::size_t size() const noexcept = 0;

        OPENTXS_EXPORT virtual ~HDAccounts() = default;
    };
    struct ImportedAccounts {
        using value_type = Imported;
        using const_iterator = opentxs::iterator::
            Bidirectional<const ImportedAccounts, const value_type>;

        /// Throws std::out_of_range for invalid position
        OPENTXS_EXPORT virtual const value_type& at(
            const std::size_t position) const noexcept(false) = 0;
        /// Throws std::out_of_range for invalid id
        OPENTXS_EXPORT virtual const value_type& at(const Identifier& id) const
            noexcept(false) = 0;
        OPENTXS_EXPORT virtual const_iterator begin() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator cbegin() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator cend() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator end() const noexcept = 0;
        OPENTXS_EXPORT virtual std::size_t size() const noexcept = 0;

        OPENTXS_EXPORT virtual ~ImportedAccounts() = default;
    };
    struct PaymentCodeAccounts {
        using value_type = PaymentCode;
        using const_iterator = opentxs::iterator::
            Bidirectional<const PaymentCodeAccounts, const value_type>;

        /// Throws std::out_of_range for invalid position
        OPENTXS_EXPORT virtual const value_type& at(
            const std::size_t position) const noexcept(false) = 0;
        /// Throws std::out_of_range for invalid id
        OPENTXS_EXPORT virtual const value_type& at(const Identifier& id) const
            noexcept(false) = 0;
        OPENTXS_EXPORT virtual const_iterator begin() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator cbegin() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator cend() const noexcept = 0;
        OPENTXS_EXPORT virtual const_iterator end() const noexcept = 0;
        OPENTXS_EXPORT virtual std::size_t size() const noexcept = 0;

        OPENTXS_EXPORT virtual ~PaymentCodeAccounts() = default;
    };

    using Element = blockchain::BalanceNode::Element;

    OPENTXS_EXPORT virtual const HDAccounts& GetHD() const noexcept = 0;
    /// Throws std::out_of_range if no keys are available
    OPENTXS_EXPORT virtual const Element& GetNextChangeKey(
        const PasswordPrompt& reason) const noexcept(false) = 0;
    /// Throws std::out_of_range if no keys are available
    OPENTXS_EXPORT virtual const Element& GetNextDepositKey(
        const PasswordPrompt& reason) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual std::string GetDepositAddress(
        const std::string& memo = "") const noexcept = 0;
    OPENTXS_EXPORT virtual std::string GetDepositAddress(
        const Identifier& contact,
        const std::string& memo = "") const noexcept = 0;
    OPENTXS_EXPORT virtual const ImportedAccounts& GetImported()
        const noexcept = 0;
    OPENTXS_EXPORT virtual const PaymentCodeAccounts& GetPaymentCode()
        const noexcept = 0;
    OPENTXS_EXPORT virtual const identifier::Nym& NymID() const noexcept = 0;
    OPENTXS_EXPORT virtual const BalanceList& Parent() const noexcept = 0;

    OPENTXS_EXPORT virtual ~BalanceTree() = default;

protected:
    BalanceTree() noexcept = default;

private:
    BalanceTree(const BalanceTree&) = delete;
    BalanceTree(BalanceTree&&) = delete;
    BalanceTree& operator=(const BalanceTree&) = delete;
    BalanceTree& operator=(BalanceTree&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCETREE_HPP
