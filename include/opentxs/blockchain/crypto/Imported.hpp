// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_IMPORTED_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_IMPORTED_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/crypto/Subaccount.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Imported : virtual public Subaccount
{
public:
    virtual ECKey Key() const = 0;

    OPENTXS_NO_EXPORT ~Imported() override = default;

protected:
    Imported() noexcept = default;

private:
    Imported(const Imported&) = delete;
    Imported(Imported&&) = delete;
    Imported& operator=(const Imported&) = delete;
    Imported& operator=(Imported&&) = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_IMPORTED_HPP
