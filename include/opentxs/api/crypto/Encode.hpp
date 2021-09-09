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
    virtual auto DataEncode(const std::string& input) const -> std::string = 0;
    virtual auto DataEncode(const Data& input) const -> std::string = 0;
    virtual auto DataDecode(const std::string& input) const -> std::string = 0;
    virtual auto IdentifierEncode(const Data& input) const -> std::string = 0;
    virtual auto IdentifierDecode(const std::string& input) const
        -> std::string = 0;
    virtual auto IsBase62(const std::string& str) const -> bool = 0;
    virtual auto Nonce(const std::uint32_t size) const -> OTString = 0;
    virtual auto Nonce(const std::uint32_t size, Data& rawOutput) const
        -> OTString = 0;
    virtual auto RandomFilename() const -> std::string = 0;
    virtual auto SanatizeBase58(const std::string& input) const
        -> std::string = 0;
    virtual auto SanatizeBase64(const std::string& input) const
        -> std::string = 0;
    virtual auto Z85Encode(const Data& input) const -> std::string = 0;
    virtual auto Z85Encode(const std::string& input) const -> std::string = 0;
    virtual auto Z85Decode(const Data& input) const -> OTData = 0;
    virtual auto Z85Decode(const std::string& input) const -> std::string = 0;

    virtual ~Encode() = default;

protected:
    Encode() = default;

private:
    Encode(const Encode&) = delete;
    Encode(Encode&&) = delete;
    auto operator=(const Encode&) -> Encode& = delete;
    auto operator=(Encode&&) -> Encode& = delete;
};
}  // namespace crypto
}  // namespace api
}  // namespace opentxs
#endif
