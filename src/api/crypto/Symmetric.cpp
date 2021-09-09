// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/crypto/Symmetric.hpp"  // IWYU pragma: associated

#include <memory>

#include "Proto.tpp"
#include "internal/api/crypto/Factory.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"

#define OT_METHOD "opentxs::api::crypto::implementation::Symmetric::"

namespace opentxs::factory
{
auto Symmetric(const api::Core& api) noexcept
    -> std::unique_ptr<api::crypto::Symmetric>
{
    using ReturnType = api::crypto::implementation::Symmetric;

    return std::make_unique<ReturnType>(api);
}
}  // namespace opentxs::factory

namespace opentxs::api::crypto::implementation
{
Symmetric::Symmetric(const api::Core& api) noexcept
    : api_(api)
{
}

auto Symmetric::GetEngine(const opentxs::crypto::key::symmetric::Algorithm mode)
    const -> const opentxs::crypto::SymmetricProvider*
{
    const opentxs::crypto::SymmetricProvider* engine{nullptr};

    // Add support for other crypto engines here
    switch (mode) {
        case opentxs::crypto::key::symmetric::Algorithm::ChaCha20Poly1305: {
            engine = &api_.Crypto().Sodium();
        } break;
        default: {
            OT_FAIL;
        }
    }

    return engine;
}

auto Symmetric::GetEngine(const opentxs::crypto::key::symmetric::Source type)
    const -> const opentxs::crypto::SymmetricProvider*
{
    switch (type) {
        case opentxs::crypto::key::symmetric::Source::Argon2i:
        case opentxs::crypto::key::symmetric::Source::Argon2id: {

            return &api_.Crypto().Sodium();
        }
        default: {

            return nullptr;
        }
    }
}

auto Symmetric::IvSize(
    const opentxs::crypto::key::symmetric::Algorithm mode) const -> std::size_t
{
    auto pEngine = GetEngine(mode);

    if (nullptr == pEngine) {
        LogOutput(OT_METHOD)(__func__)(": Invalid mode").Flush();

        return 0;
    }

    auto& engine = *pEngine;

    return engine.IvSize(mode);
}

auto Symmetric::Key(
    const PasswordPrompt& password,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return api_.Factory().SymmetricKey(*engine, password, mode);
}

auto Symmetric::Key(
    const proto::SymmetricKey& serialized,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return api_.Factory().SymmetricKey(*engine, serialized);
}

auto Symmetric::Key(
    const ReadView& serializedCiphertext,
    const opentxs::crypto::key::symmetric::Algorithm mode) const
    -> OTSymmetricKey
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    auto ciphertext = proto::Factory<proto::Ciphertext>(serializedCiphertext);

    return api_.Factory().SymmetricKey(*engine, ciphertext.key());
}

auto Symmetric::Key(
    const Secret& seed,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const opentxs::crypto::key::symmetric::Algorithm mode,
    const opentxs::crypto::key::symmetric::Source type) const -> OTSymmetricKey
{
    auto engine = GetEngine(mode);

    OT_ASSERT(nullptr != engine);

    return api_.Factory().SymmetricKey(
        *engine, seed, operations, difficulty, engine->KeySize(mode), type);
}

auto Symmetric::Key(
    const Secret& seed,
    const ReadView salt,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const std::uint64_t parallel,
    const std::size_t bytes,
    const opentxs::crypto::key::symmetric::Source type) const -> OTSymmetricKey
{
    auto engine = GetEngine(type);

    OT_ASSERT(nullptr != engine);

    return api_.Factory().SymmetricKey(
        *engine, seed, salt, operations, difficulty, parallel, bytes, type);
}
}  // namespace opentxs::api::crypto::implementation
