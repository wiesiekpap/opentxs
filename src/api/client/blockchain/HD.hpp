// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/client/blockchain/HD.cpp"

#pragma once

#include <atomic>
#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "api/client/blockchain/BalanceNode.hpp"
#include "api/client/blockchain/Deterministic.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/protobuf/HDAccount.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace proto
{
class HDAccount;
class HDPath;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class HD final : public internal::HD, public Deterministic
{
public:
    using Element = implementation::BalanceNode::Element;
    using SerializedType = proto::HDAccount;

    auto BalanceElement(const Subchain type, const Bip32Index index) const
        noexcept(false) -> const Element& final;
    auto Key(const Subchain type, const Bip32Index index) const noexcept
        -> ECKey final;
#if OT_CRYPTO_WITH_BIP32
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept -> ECKey override;
#endif  // OT_CRYPTO_WITH_BIP32

    HD(const api::internal::Core& api,
       const internal::BalanceTree& parent,
       const proto::HDPath& path,
       const PasswordPrompt& reason,
       Identifier& id)
    noexcept(false);
    HD(const api::internal::Core& api,
       const internal::BalanceTree& parent,
       const SerializedType& serialized,
       const PasswordPrompt& reason,
       Identifier& id)
    noexcept(false);

    ~HD() final = default;

private:
    using AddressMap = std::map<Bip32Index, Element>;
    using Revision = std::uint64_t;

    static const VersionNumber DefaultVersion{1};

    VersionNumber version_;
    mutable std::atomic<Revision> revision_;
    mutable AddressMap internal_addresses_;
    mutable AddressMap external_addresses_;

    static auto extract_external(
        const api::internal::Core& api,
        const client::internal::Blockchain& blockchain,
        const internal::BalanceNode& parent,
        const opentxs::blockchain::Type chain,
        const SerializedType& in) noexcept(false) -> AddressMap;
    static auto extract_incoming(const SerializedType& in)
        -> std::vector<Activity>;
    static auto extract_internal(
        const api::internal::Core& api,
        const client::internal::Blockchain& blockchain,
        const internal::BalanceNode& parent,
        const opentxs::blockchain::Type chain,
        const SerializedType& in) noexcept(false) -> AddressMap;
    static auto extract_outgoing(const SerializedType& in)
        -> std::vector<Activity>;

    auto check_activity(
        const Lock& lock,
        const std::vector<Activity>& unspent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
#if OT_CRYPTO_WITH_BIP32
    auto generate_next(
        const Lock& lock,
        const Subchain type,
        const PasswordPrompt& reason) const noexcept(false) -> Bip32Index final;
#endif  // OT_CRYPTO_WITH_BIP32
    auto mutable_element(
        const Lock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false)
        -> internal::BalanceElement& final;
    auto save(const Lock& lock) const noexcept -> bool final;
    void set_metadata(
        const Lock& lock,
        const Subchain subchain,
        const Bip32Index index,
        const Identifier& contact,
        const std::string& label) const noexcept final;

    HD(const HD&) = delete;
    HD(HD&&) = delete;
    auto operator=(const HD&) -> HD& = delete;
    auto operator=(HD&&) -> HD& = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
