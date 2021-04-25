// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/rpc/PaymentType.hpp"

#ifndef OPENTXS_RPC_SEND_PAYMENT_HPP
#define OPENTXS_RPC_SEND_PAYMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/rpc/request/Base.hpp"

namespace opentxs
{
namespace proto
{
class RPCCommand;
}  // namespace proto
}  // namespace opentxs

namespace opentxs
{
namespace rpc
{
namespace request
{
class OPENTXS_EXPORT SendPayment final : public Base
{
public:
    static auto DefaultVersion() noexcept -> VersionNumber;

    auto Amount() const noexcept -> opentxs::Amount;
    auto DestinationAccount() const noexcept -> const std::string&;
    auto Memo() const noexcept -> const std::string&;
    auto PaymentType() const noexcept -> rpc::PaymentType;
    auto RecipientContact() const noexcept -> const std::string&;
    auto SourceAccount() const noexcept -> const std::string&;

    /// throws std::runtime_error for invalid constructor arguments
    /// cheque, voucher, invoice, blinded
    SendPayment(
        SessionIndex session,
        rpc::PaymentType type,
        const std::string& sourceAccount,
        const std::string& recipientContact,
        opentxs::Amount amount,
        const std::string& memo = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    /// throws std::runtime_error for invalid constructor arguments
    /// transfer
    SendPayment(
        SessionIndex session,
        const std::string& sourceAccount,
        const std::string& recipientContact,
        const std::string& destinationAccount,
        opentxs::Amount amount,
        const std::string& memo = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    /// throws std::runtime_error for invalid constructor arguments
    /// blockchain
    SendPayment(
        SessionIndex session,
        const std::string& sourceAccount,
        const std::string& destinationAddress,
        opentxs::Amount amount,
        const std::string& recipientContact = {},
        const std::string& memo = {},
        const AssociateNyms& nyms = {}) noexcept(false);
    OPENTXS_NO_EXPORT SendPayment(const proto::RPCCommand& serialized) noexcept(
        false);
    SendPayment() noexcept;

    ~SendPayment() final;

private:
    SendPayment(const SendPayment&) = delete;
    SendPayment(SendPayment&&) = delete;
    auto operator=(const SendPayment&) -> SendPayment& = delete;
    auto operator=(SendPayment&&) -> SendPayment& = delete;
};
}  // namespace request
}  // namespace rpc
}  // namespace opentxs
#endif
