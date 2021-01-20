// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <map>
#include <optional>
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

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class Deterministic : virtual public internal::Deterministic, public BalanceNode
{
public:
#if OT_CRYPTO_WITH_BIP32
    auto GenerateNext(const Subchain type, const PasswordPrompt& reason)
        const noexcept -> std::optional<Bip32Index> final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto LastGenerated(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto LastUsed(const Subchain type) const noexcept
        -> std::optional<Bip32Index> final;
    auto Path() const noexcept -> proto::HDPath final { return path_; }
#if OT_CRYPTO_WITH_BIP32
    auto RootNode(const PasswordPrompt& reason) const noexcept -> HDKey final;
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

    static const Bip32Index Lookahead{5};
    static const Bip32Index MaxIndex{2147483648};

    const proto::HDPath path_;
#if OT_CRYPTO_WITH_BIP32
    HDKey key_;
#endif  // OT_CRYPTO_WITH_BIP32
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
#if OT_CRYPTO_WITH_BIP32
    void check_lookahead(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false);
    auto need_lookahead(const Lock& lock, const Subchain type) const noexcept
        -> bool;
    auto use_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason,
        const Identifier& contact,
        const std::string& label) const noexcept -> std::optional<Bip32Index>;
#endif  // OT_CRYPTO_WITH_BIP32

    Deterministic(
        const api::internal::Core& api,
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        const OTIdentifier id,
        std::vector<Activity> unspent,
        std::vector<Activity> spent,
        const proto::HDPath path,
        IndexMap generated,
        IndexMap used) noexcept;

private:
    auto bump(const Lock& lock, const Subchain type, IndexMap map)
        const noexcept -> std::optional<Bip32Index>;
#if OT_CRYPTO_WITH_BIP32
    virtual auto generate_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) -> Bip32Index = 0;
#endif  // OT_CRYPTO_WITH_BIP32
    virtual void set_metadata(
        const Lock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contact,
        const std::string& label) const noexcept = 0;

    Deterministic() = delete;
    Deterministic(const Deterministic&) = delete;
    Deterministic(Deterministic&&) = delete;
    auto operator=(const Deterministic&) -> Deterministic& = delete;
    auto operator=(Deterministic&&) -> Deterministic& = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
