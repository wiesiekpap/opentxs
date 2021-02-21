// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "api/client/blockchain/Deterministic.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "api/client/blockchain/BalanceNode.hpp"
#include "internal/api/Api.hpp"
#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/protobuf/Enums.pb.h"
#include "opentxs/protobuf/HDPath.pb.h"

#if OT_CRYPTO_WITH_BIP32
#define OT_METHOD                                                              \
    "opentxs::api::client::blockchain::implementation::Deterministic::"
#endif  // OT_CRYPTO_WITH_BIP32

namespace opentxs::api::client::blockchain::implementation
{
Deterministic::Deterministic(
    const api::internal::Core& api,
    const internal::BalanceTree& parent,
    const BalanceNodeType type,
    OTIdentifier&& id,
    const proto::HDPath path,
    ChainData&& data,
    Identifier& out) noexcept
    : BalanceNode(api, parent, type, std::move(id), out)
    , path_(path)
#if OT_CRYPTO_WITH_BIP32
    , key_(instantiate_key(api_, const_cast<proto::HDPath&>(path_)))
#endif  // OT_CRYPTO_WITH_BIP32
    , data_(std::move(data))
    , generated_({{data_.internal_.type_, 0}, {data_.external_.type_, 0}})
    , used_({{data_.internal_.type_, 0}, {data_.external_.type_, 0}})
{
}

Deterministic::Deterministic(
    const api::internal::Core& api,
    const internal::BalanceTree& parent,
    const BalanceNodeType type,
    const SerializedType& serialized,
    const Bip32Index generated,
    const Bip32Index used,
    ChainData&& data,
    Identifier& out) noexcept(false)
    : BalanceNode(api, parent, type, serialized.common(), out)
    , path_(serialized.path())
#if OT_CRYPTO_WITH_BIP32
    , key_(instantiate_key(api_, const_cast<proto::HDPath&>(path_)))
#endif  // OT_CRYPTO_WITH_BIP32
    , data_(std::move(data))
    , generated_(
          {{data_.internal_.type_, generated}, {data_.external_.type_, used}})
    , used_(
          {{data_.internal_.type_, serialized.internalindex()},
           {data_.external_.type_, serialized.externalindex()}})
{
}

auto Deterministic::BalanceElement(const Subchain type, const Bip32Index index)
    const noexcept(false) -> const Element&
{
    if (const auto& data = data_.internal_; data.type_ == type) {
        return data.map_.at(index);
    } else if (const auto& data = data_.external_; data.type_ == type) {
        return data.map_.at(index);
    } else {
        throw std::out_of_range("Invalid subchain");
    }
}

auto Deterministic::bump(const Lock& lock, const Subchain type, IndexMap map)
    const noexcept -> std::optional<Bip32Index>
{
    try {

        return map.at(type)++;
    } catch (...) {

        return {};
    }
}

auto Deterministic::check_activity(
    const Lock& lock,
    const std::vector<Activity>& unspent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    set_deterministic_contact(contacts);

    try {
        const auto currentExternal = used_.at(data_.external_.type_) - 1;
        const auto currentInternal = used_.at(data_.internal_.type_) - 1;
        auto targetExternal{currentExternal};
        auto targetInternal{currentInternal};

        for (const auto& [coin, key, value] : unspent) {
            const auto& [account, subchain, index] = key;

            if (const auto& data = data_.external_; data.type_ == subchain) {
                targetExternal = std::max(targetExternal, index);

                if (data.set_contact_) {
                    extract_contacts(index, data.map_, contacts);
                }
            } else if (const auto& data = data_.internal_;
                       data.type_ == subchain) {
                targetInternal = std::max(targetInternal, index);

                if (data.set_contact_) {
                    extract_contacts(index, data.map_, contacts);
                }
            } else {

                return false;
            }
        }

        const auto blank = api_.Factory().Identifier();
        const auto empty = std::string{};

#if OT_CRYPTO_WITH_BIP32
        for (auto i{currentExternal}; i < targetExternal; ++i) {
            use_next(lock, data_.external_.type_, reason, blank, empty);
        }

        for (auto i{currentInternal}; i < targetInternal; ++i) {
            use_next(lock, data_.internal_.type_, reason, blank, empty);
        }
#endif  // OT_CRYPTO_WITH_BIP32

        OT_ASSERT(targetExternal < used_.at(data_.external_.type_));
        OT_ASSERT(targetInternal < used_.at(data_.internal_.type_));

        return true;
    } catch (...) {

        return false;
    }
}

void Deterministic::check_lookahead(
    const Lock& lock,
    const PasswordPrompt& reason) const noexcept(false)
{
#if OT_CRYPTO_WITH_BIP32
    check_lookahead(lock, data_.internal_.type_, reason);
    check_lookahead(lock, data_.external_.type_, reason);
#endif  // OT_CRYPTO_WITH_BIP32
}

#if OT_CRYPTO_WITH_BIP32
void Deterministic::check_lookahead(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false)
{
    while (need_lookahead(lock, type)) { generate_next(lock, type, reason); }
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::extract_contacts(
    const Bip32Index index,
    const AddressMap& map,
    std::set<OTIdentifier>& contacts) noexcept -> void
{
    try {
        auto contact = map.at(index).Contact();

        if (false == contact->empty()) { contacts.emplace(std::move(contact)); }
    } catch (...) {
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::GenerateNext(
    const Subchain type,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    if (0 == generated_.count(type)) { return {}; }

    try {
        auto output = generate_next(lock, type, reason);

        if (save(lock)) {

            return output;
        } else {

            return {};
        }
    } catch (...) {

        return {};
    }
}
auto Deterministic::generate_next(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false) -> Bip32Index
{
    auto& index = generated_.at(type);

    if (MaxIndex <= index) { throw std::runtime_error("Account is full"); }

    auto pKey = PrivateKey(type, index, reason);

    if (false == bool(pKey)) {
        throw std::runtime_error("Failed to generate key");
    }

    const auto& key = *pKey;
    auto& addressMap = (data_.internal_.type_ == type) ? data_.internal_.map_
                                                       : data_.external_.map_;
    const auto& blockchain = parent_.Parent().Parent();
    const auto [it, added] = addressMap.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(index),
        std::forward_as_tuple(
            api_, blockchain, *this, chain_, type, index, key));

    if (false == added) { throw std::runtime_error("Failed to add key"); }

    set_deterministic_contact(it->second);
#if OT_BLOCKCHAIN
    blockchain.KeyGenerated(chain_);
#endif  // OT_BLOCKCHAIN

    return index++;
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::init(const PasswordPrompt& reason) noexcept(false) -> void
{
    Lock lock(lock_);

    if (account_already_exists(lock)) {
        throw std::runtime_error("Account already exists");
    }

    check_lookahead(lock, reason);

    if (false == save(lock)) {
        throw std::runtime_error("Failed to save new account");
    }

    init();
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::instantiate_key(
    const api::internal::Core& api,
    proto::HDPath& path) -> HDKey
{
    std::string fingerprint{path.root()};
    auto reason = api.Factory().PasswordPrompt("Loading account xpriv");
    api::HDSeed::Path children{};

    for (const auto& index : path.child()) { children.emplace_back(index); }

    auto pKey = api.Seeds().GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
        children,
        reason,
        proto::KEYROLE_SIGN,
        opentxs::crypto::key::EllipticCurve::MaxVersion);

    OT_ASSERT(pKey);

    path.set_root(fingerprint);

    return std::move(pKey);
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::Key(const Subchain type, const Bip32Index index)
    const noexcept -> ECKey
{
    try {
        if (const auto& data = data_.internal_; data.type_ == type) {

            return data.map_.at(index).Key();
        } else if (const auto& data = data_.external_; data.type_ == type) {

            return data_.external_.map_.at(index).Key();
        } else {
            return nullptr;
        }
    } catch (...) {

        return nullptr;
    }
}

auto Deterministic::LastGenerated(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    try {
        auto output = generated_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

auto Deterministic::LastUsed(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    try {
        auto output = used_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

auto Deterministic::mutable_element(
    const Lock& lock,
    const Subchain type,
    const Bip32Index index) noexcept(false) -> internal::BalanceElement&
{
    if (auto& data = data_.internal_; data.type_ == type) {

        return data.map_.at(index);
    } else if (auto& data = data_.external_; data.type_ == type) {

        return data.map_.at(index);
    } else {
        throw std::out_of_range("Invalid subchain");
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::need_lookahead(const Lock& lock, const Subchain type)
    const noexcept -> bool
{
    return Lookahead > (generated_.at(type) - used_.at(type));
}

auto Deterministic::RootNode(const PasswordPrompt& reason) const noexcept
    -> HDKey
{
    auto fingerprint(path_.root());
    api::HDSeed::Path path{};

    for (const auto& child : path_.child()) { path.emplace_back(child); }

    return api_.Seeds().GetHDKey(
        fingerprint,
        EcdsaCurve::secp256k1,
        path,
        reason,
        proto::KEYROLE_SIGN,
        opentxs::crypto::key::EllipticCurve::MaxVersion);
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::serialize_deterministic(
    const Lock& lock,
    SerializedType& out) const noexcept -> void
{
    out.set_version(BlockchainDeterministicAccountDataVersion);
    serialize_common(lock, *out.mutable_common());
    *out.mutable_path() = path_;
    out.set_internalindex(used_.at(data_.internal_.type_));
    out.set_externalindex(used_.at(data_.external_.type_));
}

auto Deterministic::set_metadata(
    const Lock& lock,
    const Subchain subchain,
    const Bip32Index index,
    const Identifier& contact,
    const std::string& label) const noexcept -> void
{
    const auto blank = api_.Factory().Identifier();

    try {
        if (auto& in = data_.internal_; in.type_ == subchain) {
            in.map_.at(index).SetMetadata(
                in.set_contact_ ? contact : blank.get(), label);
        } else if (auto& ex = data_.external_; ex.type_ == subchain) {
            ex.map_.at(index).SetMetadata(
                ex.set_contact_ ? contact : blank.get(), label);
        } else {
        }
    } catch (...) {
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::UseNext(
    const Subchain type,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label) const noexcept -> std::optional<Bip32Index>
{
    Lock lock(lock_);

    return use_next(lock, type, reason, contact, label);
}

auto Deterministic::use_next(
    const Lock& lock,
    const Subchain type,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label) const noexcept -> std::optional<Bip32Index>
{
    try {
        auto& next = used_.at(type);

        if (MaxIndex < next) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Account is full").Flush();

            return {};
        }

        auto output = next++;
        check_lookahead(lock, type, reason);
        set_metadata(lock, type, output, contact, label);

        if (save(lock)) {

            return output;
        } else {

            return {};
        }
    } catch (...) {

        return {};
    }
}
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs::api::client::blockchain::implementation
