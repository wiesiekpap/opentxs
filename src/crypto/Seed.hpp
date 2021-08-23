// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Types.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStrength.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Symmetric;
}  // namespace crypto

namespace storage
{
class Storage;
}  // namespace storage

class Core;
class Factory;
}  // namespace api

namespace crypto
{
class Bip32;
class Bip39;
}  // namespace crypto

namespace proto
{
class Seed;
}  // namespace proto

class Identifier;
class PasswordPrompt;
class Secret;
}  // namespace opentxs

namespace opentxs::crypto
{
class Seed
{
public:
    static auto Translate(int proto) noexcept -> SeedStyle;

    auto Entropy() const noexcept -> const Secret&;
    auto ID() const noexcept -> const Identifier&;
    auto Index() const noexcept -> Bip32Index;
    auto Phrase() const noexcept -> const Secret&;
    auto Type() const noexcept -> SeedStyle;
    auto Words() const noexcept -> const Secret&;

    auto IncrementIndex(const Bip32Index index) noexcept -> bool;

    Seed(
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::Factory& factory,
        const api::storage::Storage& storage,
        const Language lang,
        const SeedStrength strength,
        const PasswordPrompt& reason) noexcept(false);
    Seed(
        const api::Core& api,
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::Factory& factory,
        const api::storage::Storage& storage,
        const SeedStyle type,
        const Language lang,
        const Secret& words,
        const Secret& passphrase,
        const PasswordPrompt& reason) noexcept(false);
    Seed(
        const opentxs::crypto::Bip32& bip32,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::Factory& factory,
        const api::storage::Storage& storage,
        const Secret& entropy,
        const PasswordPrompt& reason) noexcept(false);
    Seed(
        const api::Core& api,
        const opentxs::crypto::Bip39& bip39,
        const api::crypto::Symmetric& symmetric,
        const api::Factory& factory,
        const api::storage::Storage& storage,
        const proto::Seed& proto,
        const PasswordPrompt& reason) noexcept(false);
    Seed(Seed&&) noexcept;

    ~Seed();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Seed() = delete;
    Seed(const Seed&) = delete;
    auto operator=(const Seed&) -> Seed& = delete;
    auto operator=(Seed&&) -> Seed& = delete;
};
}  // namespace opentxs::crypto
