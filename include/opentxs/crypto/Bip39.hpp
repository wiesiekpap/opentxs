// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CRYPTO_BIP39_HPP
#define OPENTXS_CRYPTO_BIP39_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>
#include <string_view>
#include <vector>

#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api

class Secret;
}  // namespace opentxs

namespace opentxs
{
namespace crypto
{
class Bip39
{
public:
    using Suggestions = std::vector<std::string>;

    OPENTXS_EXPORT virtual Suggestions GetSuggestions(
        const Language lang,
        const std::string_view word) const noexcept = 0;
    OPENTXS_EXPORT virtual std::size_t LongestWord(
        const Language lang) const noexcept = 0;
    OPENTXS_EXPORT virtual bool SeedToWords(
        const Secret& seed,
        Secret& words,
        const Language lang) const noexcept = 0;
    OPENTXS_EXPORT virtual bool WordsToSeed(
        const api::Core& api,
        const SeedStyle type,
        const Language lang,
        const Secret& words,
        Secret& seed,
        const Secret& passphrase) const noexcept = 0;

    OPENTXS_EXPORT virtual ~Bip39() = default;

protected:
    Bip39() = default;

private:
    Bip39(const Bip39&) = delete;
    Bip39(Bip39&&) = delete;
    Bip39& operator=(const Bip39&) = delete;
    Bip39& operator=(Bip39&&) = delete;
};
}  // namespace crypto
}  // namespace opentxs
#endif
