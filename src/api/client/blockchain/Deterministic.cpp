// Copyright (c) 2010-2021 The Open-Transactions developers
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
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
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
    const Bip32Index internal,
    const Bip32Index external,
    ChainData&& data,
    Identifier& out) noexcept(false)
    : BalanceNode(api, parent, type, serialized.common(), out)
    , path_(serialized.path())
#if OT_CRYPTO_WITH_BIP32
    , key_(instantiate_key(api_, const_cast<proto::HDPath&>(path_)))
#endif  // OT_CRYPTO_WITH_BIP32
    , data_(std::move(data))
    , generated_(
          {{data_.internal_.type_, internal},
           {data_.external_.type_, external}})
    , used_(
          {{data_.internal_.type_, serialized.internalindex()},
           {data_.external_.type_, serialized.externalindex()}})
{
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::accept(
    const rLock& lock,
    const Subchain type,
    const std::size_t preallocate,
    const Identifier& contact,
    const std::string& label,
    const Time time,
    const Bip32Index index,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
    check_lookahead(lock, type, preallocate, reason);
    set_metadata(lock, type, index, contact, label);
    auto& element =
        const_cast<Deterministic&>(*this).element(lock, type, index);
    element.Reserve(time);

    if (save(lock)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Accepted index ")(index).Flush();

        return index;
    } else {

        return std::nullopt;
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::BalanceElement(const Subchain type, const Bip32Index index)
    const noexcept(false) -> const Element&
{
    auto lock = rLock{lock_};

    return element(lock, type, index);
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::check(
    const rLock& lock,
    const Subchain type,
    const std::size_t preallocate,
    const Identifier& contact,
    const std::string& label,
    const Time time,
    const Bip32Index candidate,
    const PasswordPrompt& reason,
    Fallback& fallback,
    std::size_t& gap) const noexcept(false) -> std::optional<Bip32Index>
{
    using Status = Element::Availability;
    const auto accept = [&](Bip32Index index) {
        return this->accept(
            lock, type, preallocate, contact, label, time, index, reason);
    };

    if (is_generated(lock, type, candidate)) {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Examining generated index ")(
            candidate)
            .Flush();
        const auto& element = this->element(lock, type, candidate);
        const auto status = element.IsAvailable(contact, label);

        switch (status) {
            case Status::NeverUsed: {
                LogTrace(OT_METHOD)(__FUNCTION__)(": index ")(candidate)(
                    " was never used")
                    .Flush();

                return accept(candidate);
            }
            case Status::Reissue: {
                LogTrace(OT_METHOD)(__FUNCTION__)(": Recycling unused index ")(
                    candidate)
                    .Flush();

                return accept(candidate);
            }
            case Status::Used: {
                LogTrace(OT_METHOD)(__FUNCTION__)(": index ")(candidate)(
                    " has confirmed transactions")
                    .Flush();
                gap = 0;

                throw std::runtime_error("Not acceptable");
            }
            case Status::MetadataConflict: {
                LogTrace(OT_METHOD)(__FUNCTION__)(": index ")(candidate)(
                    " can not be used")
                    .Flush();
                ++gap;

                throw std::runtime_error("Not acceptable");
            }
            case Status::Reserved: {
                LogTrace(OT_METHOD)(__FUNCTION__)(": index ")(candidate)(
                    " is reserved")
                    .Flush();
                ++gap;

                throw std::runtime_error("Not acceptable");
            }
            case Status::StaleUnconfirmed:
            default: {
                LogTrace(OT_METHOD)(__FUNCTION__)(": saving index ")(candidate)(
                    " as a fallback")
                    .Flush();
                fallback[status].emplace(candidate);
                ++gap;

                throw std::runtime_error("Not acceptable");
            }
        }
    } else {
        LogTrace(OT_METHOD)(__FUNCTION__)(": Generating index ")(candidate)
            .Flush();
        generate(lock, type, candidate, reason);

        return accept(candidate);
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::check_activity(
    const rLock& lock,
    const std::vector<Activity>& unspent,
    std::set<OTIdentifier>& contacts,
    const PasswordPrompt& reason) const noexcept -> bool
{
    set_deterministic_contact(contacts);

    try {
        for (const auto& [coin, key, value] : unspent) {
            const auto& [account, subchain, index] = key;

            if (const auto& data = data_.external_; data.type_ == subchain) {

                if (data.set_contact_) {
                    extract_contacts(index, data.map_, contacts);
                }
            } else if (const auto& data = data_.internal_;
                       data.type_ == subchain) {

                if (data.set_contact_) {
                    extract_contacts(index, data.map_, contacts);
                }
            } else {

                return false;
            }
        }

        return true;
    } catch (...) {

        return false;
    }
}

void Deterministic::check_lookahead(
    const rLock& lock,
    const PasswordPrompt& reason) const noexcept(false)
{
#if OT_CRYPTO_WITH_BIP32
    check_lookahead(lock, data_.internal_.type_, 0, reason);
    check_lookahead(lock, data_.external_.type_, 0, reason);
#endif  // OT_CRYPTO_WITH_BIP32
}

#if OT_CRYPTO_WITH_BIP32
void Deterministic::check_lookahead(
    const rLock& lock,
    const Subchain type,
    const std::size_t preallocate,
    const PasswordPrompt& reason) const noexcept(false)
{
    while (need_lookahead(lock, type, preallocate)) {
        generate_next(lock, type, reason);
    }
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::confirm(
    const rLock& lock,
    const Subchain type,
    const Bip32Index index) noexcept -> void
{
    try {
        auto& used = used_.at(type);

        if (index < used) { return; }

        using Status = Element::Availability;
        static const auto blank = api_.Factory().Identifier();

        for (auto i{index}; i > used; --i) {
            const auto& element = this->element(lock, type, i - 1u);

            if (Status::Used != element.IsAvailable(blank, "")) { return; }
        }

        for (auto i{index}; i < generated_.at(type); ++i) {
            const auto& element = this->element(lock, type, i);

            if (Status::Used == element.IsAvailable(blank, "")) {
                used = std::max(used, i + 1u);
            } else {

                return;
            }
        }
    } catch (...) {
    }
}

auto Deterministic::element(
    const rLock&,
    const Subchain type,
    const Bip32Index index) noexcept(false) -> Element&
{
    if (auto& data = data_.internal_; data.type_ == type) {
        return data.map_.at(index);
    } else if (auto& data = data_.external_; data.type_ == type) {
        return data.map_.at(index);
    } else {
        throw std::out_of_range("Invalid subchain");
    }
}

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

auto Deterministic::Floor(const Subchain type) const noexcept
    -> std::optional<Bip32Index>
{
    auto lock = rLock{lock_};

    try {

        return used_.at(type);
    } catch (...) {

        return std::nullopt;
    }
}

auto Deterministic::GenerateNext(
    const Subchain type,
    const PasswordPrompt& reason) const noexcept -> std::optional<Bip32Index>
{
#if OT_CRYPTO_WITH_BIP32
    auto lock = rLock{lock_};

    if (0 == generated_.count(type)) { return {}; }

    try {
        auto output = generate_next(lock, type, reason);

        if (save(lock)) {

            return output;
        } else {

            return std::nullopt;
        }
    } catch (...) {

        return std::nullopt;
    }
#else

    return std::nullopt;
#endif  // OT_CRYPTO_WITH_BIP32
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::generate(
    const rLock& lock,
    const Subchain type,
    const Bip32Index desired,
    const PasswordPrompt& reason) const noexcept(false) -> Bip32Index
{
    auto& index = generated_.at(type);

    OT_ASSERT(desired == index);

    if (max_index_ <= index) { throw std::runtime_error("Account is full"); }

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

auto Deterministic::generate_next(
    const rLock& lock,
    const Subchain type,
    const PasswordPrompt& reason) const noexcept(false) -> Bip32Index
{
    return generate(lock, type, generated_.at(type), reason);
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::init(const PasswordPrompt& reason) noexcept(false) -> void
{
    auto lock = rLock{lock_};

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
    auto lock = rLock{lock_};

    try {
        auto output = generated_.at(type);

        return (0 == output) ? std::optional<Bip32Index>{} : output - 1;
    } catch (...) {

        return {};
    }
}

auto Deterministic::mutable_element(
    const rLock& lock,
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
auto Deterministic::need_lookahead(
    const rLock& lock,
    const Subchain type,
    const std::size_t preallocate) const noexcept -> bool
{
    return std::max<Bip32Index>(window_, preallocate) >
           (generated_.at(type) - used_.at(type));
}
#endif  // OT_CRYPTO_WITH_BIP32

auto Deterministic::Reserve(
    const Subchain type,
    const std::size_t preallocate,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label,
    const Time time) const noexcept -> std::optional<Bip32Index>
{
#if OT_CRYPTO_WITH_BIP32
    auto lock = rLock{lock_};

    return use_next(lock, type, preallocate, reason, contact, label, time);
#else

    return std::nullopt;
#endif  // OT_CRYPTO_WITH_BIP32
}

auto Deterministic::RootNode(const PasswordPrompt& reason) const noexcept
    -> HDKey
{
#if OT_CRYPTO_WITH_BIP32
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
#else

    return {};
#endif  // OT_CRYPTO_WITH_BIP32
}

auto Deterministic::serialize_deterministic(
    const rLock& lock,
    SerializedType& out) const noexcept -> void
{
    out.set_version(BlockchainDeterministicAccountDataVersion);
    serialize_common(lock, *out.mutable_common());
    *out.mutable_path() = path_;
    out.set_internalindex(used_.at(data_.internal_.type_));
    out.set_externalindex(used_.at(data_.external_.type_));
}

auto Deterministic::set_metadata(
    const rLock& lock,
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

auto Deterministic::unconfirm(
    const rLock& lock,
    const Subchain type,
    const Bip32Index index) noexcept -> void
{
    try {
        auto& used = used_.at(type);
        const auto& element = this->element(lock, type, index);

        if (0 == element.Confirmed().size()) { used = std::min(used, index); }
    } catch (...) {
    }
}

#if OT_CRYPTO_WITH_BIP32
auto Deterministic::use_next(
    const rLock& lock,
    const Subchain type,
    const std::size_t preallocate,
    const PasswordPrompt& reason,
    const Identifier& contact,
    const std::string& label,
    const Time time) const noexcept -> std::optional<Bip32Index>
{
    try {
        auto gap = std::size_t{0};
        auto candidate = used_.at(type);
        auto fallback = Fallback{};

        while (gap < window_) {
            try {

                return check(
                    lock,
                    type,
                    preallocate,
                    contact,
                    label,
                    time,
                    candidate,
                    reason,
                    fallback,
                    gap);
            } catch (...) {
                ++candidate;

                continue;
            }
        }

        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": Gap limit reached. Searching for acceptable fallback")
            .Flush();
        const auto accept = [&](Bip32Index index) {
            return this->accept(
                lock, type, preallocate, contact, label, time, index, reason);
        };

        if (auto& set = fallback[Status::StaleUnconfirmed]; 0 < set.size()) {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Recycling index with old never-confirmed transactions")
                .Flush();

            return accept(*set.cbegin());
        }

        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": No acceptable fallback discovered. Generating past the gap "
            "limit")
            .Flush();

        while (candidate < max_index_) {
            try {

                return check(
                    lock,
                    type,
                    preallocate,
                    contact,
                    label,
                    time,
                    candidate,
                    reason,
                    fallback,
                    gap);
            } catch (...) {
                ++candidate;

                continue;
            }
        }

        return {};
    } catch (...) {

        return {};
    }
}
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace opentxs::api::client::blockchain::implementation
