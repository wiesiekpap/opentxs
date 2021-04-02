// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CRYPTO_ENCODE_HPP
#define OPENTXS_API_CRYPTO_ENCODE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Encode
{
public:
    virtual std::string DataEncode(const std::string& input) const = 0;
    virtual std::string DataEncode(const Data& input) const = 0;
    virtual std::string DataDecode(const std::string& input) const = 0;
    virtual std::string IdentifierEncode(const Data& input) const = 0;
    virtual std::string IdentifierDecode(const std::string& input) const = 0;
    virtual bool IsBase62(const std::string& str) const = 0;
    virtual OTString Nonce(const std::uint32_t size) const = 0;
    virtual OTString Nonce(const std::uint32_t size, Data& rawOutput) const = 0;
    virtual std::string RandomFilename() const = 0;
    virtual std::string SanatizeBase58(const std::string& input) const = 0;
    virtual std::string SanatizeBase64(const std::string& input) const = 0;
    virtual std::string Z85Encode(const Data& input) const = 0;
    virtual std::string Z85Encode(const std::string& input) const = 0;
    virtual OTData Z85Decode(const Data& input) const = 0;
    virtual std::string Z85Decode(const std::string& input) const = 0;

    virtual ~Encode() = default;

protected:
    Encode() = default;

private:
    Encode(const Encode&) = delete;
    Encode(Encode&&) = delete;
    Encode& operator=(const Encode&) = delete;
    Encode& operator=(Encode&&) = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif
