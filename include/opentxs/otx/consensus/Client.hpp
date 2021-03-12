// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_OTX_CONSENSUS_CLIENT_HPP
#define OPENTXS_OTX_CONSENSUS_CLIENT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/otx/consensus/Base.hpp"

namespace opentxs
{
namespace otx
{
namespace context
{
class TransactionStatement;
}  // namespace context
}  // namespace otx
}  // namespace opentxs

namespace opentxs
{
namespace otx
{
namespace context
{
class Client : virtual public Base
{
public:
    OPENTXS_EXPORT virtual bool hasOpenTransactions() const = 0;
    using Base::IssuedNumbers;
    OPENTXS_EXPORT virtual std::size_t IssuedNumbers(
        const TransactionNumbers& exclude) const = 0;
    OPENTXS_EXPORT virtual std::size_t OpenCronItems() const = 0;
    OPENTXS_EXPORT virtual bool Verify(
        const TransactionStatement& statement,
        const TransactionNumbers& excluded,
        const TransactionNumbers& included) const = 0;
    OPENTXS_EXPORT virtual bool VerifyCronItem(
        const TransactionNumber number) const = 0;
    using Base::VerifyIssuedNumber;
    OPENTXS_EXPORT virtual bool VerifyIssuedNumber(
        const TransactionNumber& number,
        const TransactionNumbers& exclude) const = 0;

    OPENTXS_EXPORT virtual bool AcceptIssuedNumbers(
        TransactionNumbers& newNumbers) = 0;
    OPENTXS_EXPORT virtual void FinishAcknowledgements(
        const RequestNumbers& req) = 0;
    OPENTXS_EXPORT virtual bool IssueNumber(
        const TransactionNumber& number) = 0;

    OPENTXS_EXPORT ~Client() override = default;

protected:
    Client() = default;

private:
    Client(const Client&) = delete;
    Client(Client&&) = delete;
    Client& operator=(const Client&) = delete;
    Client& operator=(Client&&) = delete;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
#endif
