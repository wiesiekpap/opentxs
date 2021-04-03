// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_PAYMENTCODE_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_PAYMENTCODE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/api/client/blockchain/Deterministic.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class PaymentCode : virtual public Deterministic
{
public:
    using Txid = opentxs::blockchain::block::Txid;

    OPENTXS_EXPORT virtual bool AddNotification(
        const Txid& tx) const noexcept = 0;
    OPENTXS_EXPORT virtual bool IsNotified() const noexcept = 0;
    OPENTXS_EXPORT virtual const opentxs::PaymentCode& Local()
        const noexcept = 0;
    OPENTXS_EXPORT virtual bool ReorgNotification(
        const Txid& tx) const noexcept = 0;
    OPENTXS_EXPORT virtual const opentxs::PaymentCode& Remote()
        const noexcept = 0;

    OPENTXS_EXPORT ~PaymentCode() override = default;

protected:
    PaymentCode() noexcept = default;

private:
    PaymentCode(const PaymentCode&) = delete;
    PaymentCode(PaymentCode&&) = delete;
    PaymentCode& operator=(const PaymentCode&) = delete;
    PaymentCode& operator=(PaymentCode&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_PAYMENTCODECHAIN_HPP
