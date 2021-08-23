// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_HD_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_HD_HPP

// IWYU pragma: no_include "opentxs/blockchain/crypto/HDProtocol.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT HD : virtual public Deterministic
{
public:
    virtual auto Name() const noexcept -> std::string = 0;
    virtual auto Standard() const noexcept -> HDProtocol = 0;

    OPENTXS_NO_EXPORT ~HD() override = default;

protected:
    HD() noexcept = default;

private:
    HD(const HD&) = delete;
    HD(HD&&) = delete;
    HD& operator=(const HD&) = delete;
    HD& operator=(HD&&) = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_HDCHAIN_HPP
