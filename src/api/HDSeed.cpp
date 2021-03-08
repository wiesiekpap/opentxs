// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/HDSeed.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "internal/api/Api.hpp"
#include "internal/api/Factory.hpp"
#include "internal/api/crypto/Crypto.hpp"
#include "internal/crypto/key/Factory.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/Primitives.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip39.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Bip44Type.hpp"
#include "opentxs/crypto/Language.hpp"      // IWYU pragma: keep
#include "opentxs/crypto/SeedStrength.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/SeedStyle.hpp"     // IWYU pragma: keep
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/Ciphertext.pb.h"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/HDPath.pb.h"
#include "opentxs/protobuf/Seed.pb.h"
#include "opentxs/protobuf/verify/Seed.hpp"
#include "util/Container.hpp"
#include "util/HDIndex.hpp"  // IWYU pragma: keep

#define OT_METHOD "opentxs::api::implementation::HDSeed::"

namespace opentxs::factory
{
auto HDSeed(
    const api::Factory& factory,
    const api::crypto::internal::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::storage::Storage& storage,
    const crypto::Bip32& bip32,
    const crypto::Bip39& bip39) noexcept -> std::unique_ptr<api::HDSeed>
{
    using ReturnType = api::implementation::HDSeed;

    return std::make_unique<ReturnType>(
        factory, asymmetric, symmetric, storage, bip32, bip39);
}
}  // namespace opentxs::factory

namespace opentxs::api::implementation
{
const proto::SymmetricMode HDSeed::DEFAULT_ENCRYPTION_MODE =
    proto::SMODE_CHACHA20POLY1305;
const std::string HDSeed::DEFAULT_PASSPHRASE = "";
const HDSeed::SeedMap HDSeed::seed_map_{
    {Style::BIP32, proto::SEEDTYPE_RAW},
    {Style::BIP39, proto::SEEDTYPE_BIP39},
};
const HDSeed::SeedReverseMap HDSeed::seed_reverse_map_{reverse_map(seed_map_)};
const HDSeed::LangMap HDSeed::lang_map_{
    {Language::none, proto::SEEDLANG_NONE},
    {Language::en, proto::SEEDLANG_EN},
};
const HDSeed::LangReverseMap HDSeed::lang_reverse_map_{reverse_map(lang_map_)};

HDSeed::HDSeed(
    [[maybe_unused]] const api::Factory& factory,
    [[maybe_unused]] const api::crypto::internal::Asymmetric& asymmetric,
    const api::crypto::Symmetric& symmetric,
    const api::storage::Storage& storage,
    const opentxs::crypto::Bip32& bip32,
    const opentxs::crypto::Bip39& bip39)
    : symmetric_(symmetric)
#if OT_CRYPTO_WITH_BIP32
    , asymmetric_(asymmetric)
#endif  // OT_CRYPTO_WITH_BIP32
    , storage_(storage)
    , bip32_(bip32)
    , bip39_(bip39)
    , binary_secret_(opentxs::Context().Factory().Secret(0))
    , text_secret_(opentxs::Context().Factory().Secret(0))
    , lock_()
{
}

#if OT_CRYPTO_WITH_BIP32
auto HDSeed::AccountChildKey(
    const proto::HDPath& rootPath,
    const BIP44Chain internal,
    const Bip32Index index,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    std::string fingerprint{rootPath.root()};
    const Bip32Index change = internal ? 1 : 0;
    Path path{};

    for (const auto& child : rootPath.child()) { path.emplace_back(child); }

    path.emplace_back(change);
    path.emplace_back(index);

    return GetHDKey(fingerprint, EcdsaCurve::secp256k1, path, reason);
}
#endif  // OT_CRYPTO_WITH_BIP32

auto HDSeed::AllowedSeedTypes() const noexcept -> const SupportedSeeds&
{
    static const auto map = SupportedSeeds{
        {Style::BIP39, "BIP-39"},
    };

    return map;
}

auto HDSeed::AllowedLanguages(const Style type) const noexcept
    -> const SupportedLanguages&
{
    static const auto null = SupportedLanguages{};
    static const auto map = std::map<Style, SupportedLanguages>{
        {Style::BIP39,
         {
             {Language::en, "English"},
         }},
    };

    try {
        return map.at(type);
    } catch (...) {

        return null;
    }
}

auto HDSeed::AllowedSeedStrength(const Style type) const noexcept
    -> const SupportedStrengths&
{
    static const auto null = SupportedStrengths{};
    static const auto map = std::map<Style, SupportedStrengths>{
        {Style::BIP39,
         {
             {Strength::Twelve, "12"},
             {Strength::Fifteen, "15"},
             {Strength::Eighteen, "18"},
             {Strength::TwentyOne, "21"},
             {Strength::TwentyFour, "24"},
         }},
    };

    try {
        return map.at(type);
    } catch (...) {

        return null;
    }
}

auto HDSeed::Bip32Root(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const -> std::string
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed = 0;
    const auto seed = Seed(input, notUsed, reason);

    if (seed->empty()) { return ""; }

    auto start = reinterpret_cast<const unsigned char*>(seed->data());
    const auto end = start + seed->size();

    std::vector<unsigned char> bytes(start, end);
    std::ostringstream stream;
    stream << std::hex << std::setfill('0');

    for (int byte : bytes) { stream << std::setw(2) << byte; }

    return stream.str();
}

auto HDSeed::decrypt_seed(
    const proto::Seed& seed,
    Secret& words,
    Secret& phrase,
    Secret& raw,
    const PasswordPrompt& reason) const -> bool
{
    if (false == proto::Validate(seed, VERBOSE)) { return false; }

    const auto& cwords = seed.words();
    const auto& cphrase = seed.passphrase();
    const auto& craw = seed.raw();
    const auto& session = (3 > seed.version()) ? cwords : craw;
    auto key = symmetric_.Key(session.key(), session.mode());
    bool decrypted{false};

    OT_ASSERT(key.get());

    if (seed.has_words()) {
        decrypted =
            key->Decrypt(cwords, reason, words.WriteInto(Secret::Mode::Text));

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt words.")
                .Flush();

            return false;
        }
    }

    if (seed.has_passphrase()) {
        decrypted =
            key->Decrypt(cphrase, reason, phrase.WriteInto(Secret::Mode::Text));

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to decrypt passphrase.")
                .Flush();

            return false;
        }
    }

    if (seed.has_raw()) {
        decrypted =
            key->Decrypt(craw, reason, raw.WriteInto(Secret::Mode::Mem));

        if (false == decrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to decrypt entropy.")
                .Flush();

            return false;
        }
    }

    return true;
}

auto HDSeed::DefaultSeed() const -> std::string
{
    return storage_.DefaultSeed();
}

#if OT_CRYPTO_WITH_BIP32
auto HDSeed::GetHDKey(
    std::string& fingerprint,
    const EcdsaCurve& curve,
    const Path& path,
    const PasswordPrompt& reason,
    const proto::KeyRole role,
    const VersionNumber version) const
    -> std::unique_ptr<opentxs::crypto::key::HD>
{
    Bip32Index notUsed{0};
    auto seed = Seed(fingerprint, notUsed, reason);

    if (seed->empty()) { return {}; }

    return asymmetric_.NewHDKey(
        fingerprint, seed, curve, path, reason, role, version);
}

#endif  // OT_CRYPTO_WITH_BIP32
auto HDSeed::GetOrCreateDefaultSeed(
    std::string& fingerprint,
    Style& type,
    Language& lang,
    Bip32Index& index,
    const Strength strength,
    const PasswordPrompt& reason) const -> OTSecret
{
    Lock lock(lock_);
    fingerprint = DefaultSeed();

    if (fingerprint.empty()) {
        fingerprint = NewSeed(type, lang, strength, reason);
    }

    if (fingerprint.empty()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to create seed.").Flush();

        return {binary_secret_};
    }

    return load_seed(fingerprint, index, type, lang, reason);
}

#if OT_CRYPTO_WITH_BIP32
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
auto HDSeed::GetPaymentCode(
    std::string& fingerprint,
    const Bip32Index nym,
    const std::uint8_t version,
    const PasswordPrompt& reason) const
    -> std::unique_ptr<opentxs::crypto::key::Secp256k1>
{
    Bip32Index notUsed{0};
    auto seed = Seed(fingerprint, notUsed, reason);

    if (seed->empty()) { return {}; }

    auto pKey = asymmetric_.NewSecp256k1Key(
        fingerprint,
        seed,
        {HDIndex{Bip43Purpose::PAYCODE, Bip32Child::HARDENED},
         HDIndex{Bip44Type::BITCOIN, Bip32Child::HARDENED},
         HDIndex{nym, Bip32Child::HARDENED}},
        reason);

    if (!pKey) { return pKey; }

    const auto& key = *pKey;

    switch (version) {
        case 3: {
        } break;
        case 1:
        case 2:
        default: {
            return pKey;
        }
    }

    const auto& api = asymmetric_.API();
    const auto code = [&] {
        auto out = api.Factory().Secret(0);
        api.Crypto().Hash().Digest(
            proto::HASHTYPE_SHA256D, key.PublicKey(), out->WriteInto());

        return out;
    }();
    const auto path = [&] {
        auto out = proto::HDPath{};
        key.Path(out);

        return out;
    }();

    return factory::Secp256k1Key(
        api,
        api.Crypto().SECP256K1(),
        api.Factory().SecretFromBytes(key.PrivateKey(reason)),
        code,
        api.Factory().Data(key.PublicKey()),
        path,
        key.Parent(),
        key.Role(),
        key.Version(),
        reason);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

auto HDSeed::GetStorageKey(
    std::string& fingerprint,
    const PasswordPrompt& reason) const -> OTSymmetricKey
{
    auto pKey = GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
        {HDIndex{Bip43Purpose::FS, Bip32Child::HARDENED},
         HDIndex{Bip32Child::ENCRYPT_KEY, Bip32Child::HARDENED}},
        reason);

    if (false == bool(pKey)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive storage key.")
            .Flush();

        return OTSymmetricKey{opentxs::factory::SymmetricKey()};
    }

    const auto& key = *pKey;

    return symmetric_.Key(
        opentxs::Context().Factory().SecretFromBytes(key.PrivateKey(reason)));
}
#endif  // OT_CRYPTO_WITH_BIP32

auto HDSeed::ImportRaw(const Secret& entropy, const PasswordPrompt& reason)
    const -> std::string
{
    if (16 > entropy.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Entropy too short (")(
            entropy.size())(")")
            .Flush();

        return {};
    }

    if (64 < entropy.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Entropy too long (")(
            entropy.size())(")")
            .Flush();

        return {};
    }

    return save_seed(
        text_secret_,
        text_secret_,
        entropy,
        Style::BIP32,
        Language::none,
        reason);
}

auto HDSeed::ImportSeed(
    const Secret& words,
    const Secret& passphrase,
    const Style type,
    const Language lang,
    const PasswordPrompt& reason) const -> std::string
{
    return save_seed(words, passphrase, binary_secret_, type, lang, reason);
}

auto HDSeed::load_seed(
    std::string& fingerprint,
    Bip32Index& index,
    Style& type,
    Language& lang,
    const PasswordPrompt& reason) const -> OTSecret
{
    auto serialized = serialized_seed(fingerprint, index, reason);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(
            fingerprint)(")")
            .Flush();

        return {binary_secret_};
    }

    if (4 > serialized->version()) {
        if (serialized->has_words()) {
            type = Style::BIP39;
        } else {
            type = Style::BIP32;
        }

        lang = Language::en;
    } else {
        type = translate(serialized->type());
        lang = translate(serialized->lang());
    }

    auto seed{binary_secret_};
    auto words{text_secret_}, phrase{text_secret_}, raw{binary_secret_};

    if (false == decrypt_seed(*serialized, words, phrase, raw, reason)) {
        return {binary_secret_};
    }

    const auto extracted = seed_to_data(words, phrase, raw, seed);

    if (extracted) { return seed; }

    return {binary_secret_};
}

auto HDSeed::LongestWord(const Style type, const Language lang) const noexcept
    -> std::size_t
{
    if (Style::BIP39 != type) { return {}; }

    return bip39_.LongestWord(lang);
}

auto HDSeed::NewSeed(
    const Style type,
    const Language lang,
    const Strength strength,
    const PasswordPrompt& reason) const -> std::string
{
    switch (type) {
        case Style::BIP39: {
            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported seed type")
                .Flush();

            return {};
        }
    }

    auto entropy{binary_secret_}, words{text_secret_}, phrase{text_secret_};
    static constexpr auto bitsPerByte{8u};
    entropy->Randomize(static_cast<std::size_t>(strength) / bitsPerByte);
    phrase->AssignText(DEFAULT_PASSPHRASE);

    if (false == bip39_.SeedToWords(entropy, words, lang)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Unable to convert entropy to word list.")
            .Flush();

        return {};
    }

    return save_seed(words, phrase, binary_secret_, type, lang, reason);
}

auto HDSeed::Passphrase(
    const PasswordPrompt& reason,
    const std::string& fingerprint) const -> std::string
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed = 0;
    auto seed = serialized_seed(input, notUsed, reason);

    if (false == bool(seed)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(
            fingerprint)(")")
            .Flush();

        return {};
    }

    auto words{text_secret_}, phrase{text_secret_}, raw{binary_secret_};

    if (!decrypt_seed(*seed, words, phrase, raw, reason)) { return ""; }

    return std::string{phrase->Bytes()};
}

auto HDSeed::save_seed(
    const Secret& words,
    const Secret& passphrase,
    const Secret& raw,
    const Style type,
    const Language lang,
    const PasswordPrompt& reason) const -> std::string
{
    auto seed{binary_secret_};
    const bool haveRaw = (0 < raw.size());
    const bool haveWords = (0 < words.size());
    const bool havePhrase = (0 < passphrase.size());

    if (haveRaw) {
        seed = raw;
    } else {
        bip39_.WordsToSeed(words, seed, passphrase);
    }

    OT_ASSERT(0 < seed->size());

    auto fingerprint{bip32_.SeedID(seed->Bytes())->str()};
    auto key = symmetric_.Key(reason, DEFAULT_ENCRYPTION_MODE);

    OT_ASSERT(key.get());

    proto::Seed serialized;
    serialized.set_version(DefaultVersion);
    serialized.set_index(0);
    serialized.set_fingerprint(fingerprint);
    bool encrypted{false};
    auto& encryptedRaw = *serialized.mutable_raw();
    encrypted = key->Encrypt(seed->Bytes(), reason, encryptedRaw);

    if (false == encrypted) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt entropy.")
            .Flush();

        return "";
    }

    if (haveWords) {
        auto& encryptedWords = *serialized.mutable_words();
        encrypted = key->Encrypt(words.Bytes(), reason, encryptedWords, false);

        if (false == encrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to encrypt seed.")
                .Flush();

            return "";
        }
    }

    if (havePhrase) {
        auto& encryptedPassphrase = *serialized.mutable_passphrase();
        encrypted = key->Encrypt(
            passphrase.Bytes(), reason, encryptedPassphrase, false);

        if (false == encrypted) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed to encrypt passphrase.")
                .Flush();

            return "";
        }
    }

    serialized.set_type(translate(type));
    serialized.set_lang(translate(lang));
    const bool stored = storage_.Store(serialized, fingerprint);

    if (false == stored) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to store seed.").Flush();

        return "";
    }

    return fingerprint;
}

auto HDSeed::Seed(
    std::string& fingerprint,
    Bip32Index& index,
    const PasswordPrompt& reason) const -> OTSecret
{
    auto type = Style{};
    auto lang = Language{};

    return load_seed(fingerprint, index, type, lang, reason);
}

auto HDSeed::seed_to_data(
    const Secret& words,
    const Secret& passphrase,
    const Secret& raw,
    Secret& output) const -> bool
{
    if (0 < raw.size()) {
        output.Assign(raw);
    } else {
        bip39_.WordsToSeed(words, output, passphrase);
    }

    return true;
}

auto HDSeed::serialized_seed(
    std::string& fingerprint,
    Bip32Index& index,
    const PasswordPrompt& reason) const -> std::shared_ptr<proto::Seed>
{
    auto serialized = std::shared_ptr<proto::Seed>{};
    storage_.Load(fingerprint, serialized);

    if (serialized) { index = serialized->index(); }

    return serialized;
}

auto HDSeed::UpdateIndex(
    std::string& seed,
    const Bip32Index index,
    const PasswordPrompt& reason) const -> bool
{
    Bip32Index oldIndex = 0;
    auto serialized = serialized_seed(seed, oldIndex, reason);

    if (false == bool(serialized)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(seed)(")")
            .Flush();

        return false;
    }

    if (oldIndex > index) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Index values must always increase.")
            .Flush();

        return false;
    }

    serialized->set_index(index);

    if (DefaultVersion > serialized->version()) {
        serialized->set_version(DefaultVersion);
    }

    return storage_.Store(*serialized, seed);
}

auto HDSeed::ValidateWord(
    const Style type,
    const Language lang,
    const std::string_view word) const noexcept -> Matches
{
    if (Style::BIP39 != type) { return {}; }

    return bip39_.GetSuggestions(lang, word);
}

auto HDSeed::WordCount(const Style type, const Strength strength) const noexcept
    -> std::size_t
{
    if (Style::BIP39 != type) { return {}; }

    static const auto map = std::map<Strength, std::size_t>{
        {Strength::Twelve, 12},
        {Strength::Fifteen, 15},
        {Strength::Eighteen, 18},
        {Strength::TwentyOne, 21},
        {Strength::TwentyFour, 24},
    };

    try {

        return map.at(strength);
    } catch (...) {

        return {};
    }
}

auto HDSeed::Words(const PasswordPrompt& reason, const std::string& fingerprint)
    const -> std::string
{
    // TODO: make fingerprint non-const
    std::string input(fingerprint);
    Bip32Index notUsed;
    auto seed = serialized_seed(input, notUsed, reason);

    if (false == bool(seed)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to load seed (")(
            fingerprint)(")")
            .Flush();

        return {};
    }

    auto words{text_secret_}, phrase{text_secret_}, raw{binary_secret_};

    if (!decrypt_seed(*seed, words, phrase, raw, reason)) { return ""; }

    return std::string{words->Bytes()};
}
}  // namespace opentxs::api::implementation
