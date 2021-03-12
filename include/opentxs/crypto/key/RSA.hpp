// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_KEY_RSA_HPP
#define OPENTXS_CRYPTO_KEY_RSA_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#if OT_CRYPTO_SUPPORTED_KEY_RSA
#include "opentxs/crypto/key/Asymmetric.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class RSA : virtual public Asymmetric
{
public:
    OPENTXS_EXPORT ~RSA() override = default;

protected:
    RSA() = default;

private:
    RSA(const RSA&) = delete;
    RSA(RSA&&) = delete;
    RSA& operator=(const RSA&) = delete;
    RSA& operator=(RSA&&) = delete;
};
}  // namespace key
}  // namespace crypto
}  // namespace opentxs
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
#endif
