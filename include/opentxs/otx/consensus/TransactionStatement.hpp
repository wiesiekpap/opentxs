// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CONSENSUS_TRANSACTIONSTATEMENT_HPP
#define OPENTXS_CONSENSUS_TRANSACTIONSTATEMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <set>
#include <string>

#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace otx
{
namespace context
{
class TransactionStatement
{
private:
    std::string version_;
    std::string nym_id_;
    std::string notary_;
    std::set<TransactionNumber> available_;
    std::set<TransactionNumber> issued_;

    TransactionStatement() = delete;
    TransactionStatement(const TransactionStatement& rhs) = delete;
    auto operator=(const TransactionStatement& rhs)
        -> TransactionStatement& = delete;
    auto operator=(TransactionStatement&& rhs)
        -> TransactionStatement& = delete;

public:
    TransactionStatement(
        const std::string& notary,
        const std::set<TransactionNumber>& issued,
        const std::set<TransactionNumber>& available);
    TransactionStatement(const String& serialized);
    TransactionStatement(TransactionStatement&& rhs) = default;

    explicit operator OTString() const;

    auto Issued() const -> const std::set<TransactionNumber>&;
    auto Notary() const -> const std::string&;

    void Remove(const TransactionNumber& number);

    ~TransactionStatement() = default;
};
}  // namespace context
}  // namespace otx
}  // namespace opentxs
#endif
