// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_LIBRARY_ENCODINGPROVIDER_HPP
#define OPENTXS_CRYPTO_LIBRARY_ENCODINGPROVIDER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/Types.hpp"

namespace opentxs
{
namespace crypto
{
class OPENTXS_EXPORT EncodingProvider
{
public:
    virtual std::string Base58CheckEncode(
        const std::uint8_t* inputStart,
        const std::size_t& inputSize) const = 0;
    virtual bool Base58CheckDecode(const std::string&& input, RawData& output)
        const = 0;

    virtual ~EncodingProvider() = default;

protected:
    EncodingProvider() = default;

private:
    EncodingProvider(const EncodingProvider&) = delete;
    EncodingProvider(EncodingProvider&&) = delete;
    EncodingProvider& operator=(const EncodingProvider&) = delete;
    EncodingProvider& operator=(EncodingProvider&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
