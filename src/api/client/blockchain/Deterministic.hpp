// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "api/client/blockchain/BalanceNode.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/protobuf/BlockchainDeterministicAccountData.pb.h"
#include "opentxs/protobuf/HDPath.pb.h"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace proto
{
class BlockchainDeterministicAccountData;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class Deterministic : virtual public internal::Deterministic, public BalanceNode
{
public:
    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const Element& final;
#if OT_CRYPTO_WITH_BIP32
    auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> ECKey final;
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto LastUsed(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto Path() const noexcept -> proto::HDPath final { return path_; }
#if OT_CRYPTO_WITH_BIP32
    auto RootNode(const PasswordPrompt& reason) const noexcept
        -> HDKey override;
    auto UseNext(
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label) const noexcept
        -> std::optional<Bip32Index> final;
#endif  // OT_CRYPTO_WITH_BIP32

    ~Deterministic() override = default;

protected:
    using IndexMap = std::map<Subchain, Bip32Index>;
    using SerializedType = proto::BlockchainDeterministicAccountData;

    struct ChainData {
        AddressData internal_{};
        AddressData external_{};
    };

    static const Bip32Index Lookahead{5};
    static const Bip32Index MaxIndex{2147483648};

    const proto::HDPath path_;
#if OT_CRYPTO_WITH_BIP32
    HDKey key_;
#endif  // OT_CRYPTO_WITH_BIP32
    mutable ChainData data_;
    mutable IndexMap generated_;
    mutable IndexMap used_;

#if OT_CRYPTO_WITH_BIP32
    static auto instantiate_key(
        const api::internal::Core& api,
        proto::HDPath& path) -> HDKey;
#endif  // OT_CRYPTO_WITH_BIP32

    auto bump_generated(const Lock& lock, const Subchain type) const noexcept
        -> std::optional<Bip32Index>
    {
        return bump(lock, type, generated_);
    }
    auto bump_used(const Lock& lock, const Subchain type) const noexcept
        -> std::optional<Bip32Index>
    {
        return bump(lock, type, used_);
    }
    auto check_lookahead(const Lock& lock, const PasswordPrompt& reason) const
        noexcept(false) -> void;
#if OT_CRYPTO_WITH_BIP32
    auto check_lookahead(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) -> void;
    auto need_lookahead(const Lock& lock, const Subchain type) const noexcept
        -> bool;
#endif  // OT_CRYPTO_WITH_BIP32
    auto serialize_deterministic(const Lock& lock, SerializedType& out)
        const noexcept -> void;
#if OT_CRYPTO_WITH_BIP32
    auto use_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label) const noexcept -> std::optional<Bip32Index>;
#endif  // OT_CRYPTO_WITH_BIP32

    using BalanceNode::init;
    auto init(const PasswordPrompt& reason) noexcept(false) -> void;

    Deterministic(
        const api::internal::Core& api,
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        OTIdentifier&& id,
        const proto::HDPath path,
        ChainData&& data,
        Identifier& out) noexcept;
    Deterministic(
        const api::internal::Core& api,
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        const SerializedType& serialized,
        const Bip32Index generated,
        const Bip32Index used,
        ChainData&& data,
        Identifier& out) noexcept(false);

private:
    static constexpr auto BlockchainDeterministicAccountDataVersion =
        VersionNumber{1};

    static auto extract_contacts(
        const Bip32Index index,
        const AddressMap& map,
        std::set<OTIdentifier>& contacts) noexcept -> void;

    auto bump(const Lock& lock, const Subchain type, IndexMap map)
        const noexcept -> std::optional<Bip32Index>;
    auto check_activity(
        const Lock& lock,
        const std::vector<Activity>& unspent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
#if OT_CRYPTO_WITH_BIP32
    auto generate_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) -> Bip32Index;
#endif  // OT_CRYPTO_WITH_BIP32
    auto mutable_element(
        const Lock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false)
        -> internal::BalanceElement& final;
    virtual auto set_deterministic_contact(Element&) const noexcept -> void {}
    virtual auto set_deterministic_contact(
        std::set<OTIdentifier>&) const noexcept -> void
    {
    }
    auto set_metadata(
        const Lock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contact,
        const std::string& label) const noexcept -> void;

    Deterministic() = delete;
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
