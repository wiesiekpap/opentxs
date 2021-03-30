// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <robin_hood.h>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/BalanceNode.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/protobuf/BlockchainAccountData.pb.h"
#include "opentxs/protobuf/BlockchainActivity.pb.h"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
namespace internal
{
struct BalanceElement;
}  // namespace internal
}  // namespace blockchain

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

namespace crypto
{
namespace key
{
class EllipticCurve;
class HD;
}  // namespace key
}  // namespace crypto

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class AsymmetricKey;
class BlockchainAccountData;
}  // namespace proto

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::api::client::blockchain::implementation
{
class BalanceNode : virtual public internal::BalanceNode
{
public:
    auto AssociateTransaction(
        const std::vector<Activity>& unspent,
        const std::vector<Activity>& outgoing,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool final;
    auto ID() const noexcept -> const Identifier& final { return id_; }
    auto IncomingTransactions(const Key& key) const noexcept
        -> std::set<std::string> final;
    auto Parent() const noexcept -> const internal::BalanceTree& final
    {
        return parent_;
    }
    auto PrivateKey(
        const Subchain type,
        const Bip32Index index,
        const PasswordPrompt& reason) const noexcept -> ECKey override
    {
        return {};
    }

    auto Confirm(
        const Subchain type,
        const Bip32Index index,
        const Txid& tx) noexcept -> bool final;
    auto SetContact(
        const Subchain type,
        const Bip32Index index,
        const Identifier& id) noexcept(false) -> bool final;
    auto SetLabel(
        const Subchain type,
        const Bip32Index index,
        const std::string& label) noexcept(false) -> bool final;
    auto Type() const noexcept -> BalanceNodeType final { return type_; }
    auto Unconfirm(
        const Subchain type,
        const Bip32Index index,
        const Txid& tx,
        const Time time) noexcept -> bool final;
    auto Unreserve(const Subchain type, const Bip32Index index) noexcept
        -> bool final;
    auto UpdateElement(std::vector<ReadView>& pubkeyHashes) const noexcept
        -> void final;

    ~BalanceNode() override = default;

protected:
    using AddressMap = robin_hood::unordered_flat_map<
        Bip32Index,
        std::unique_ptr<internal::BalanceElement>>;
    using Revision = std::uint64_t;

    struct AddressData {
        const Subchain type_{};
        const bool set_contact_{};
        AddressMap map_{};
    };

    const api::internal::Core& api_;
    const internal::BalanceTree& parent_;
    const opentxs::blockchain::Type chain_;
    const BalanceNodeType type_;
    const OTIdentifier id_;
    mutable std::recursive_mutex lock_;
    mutable std::atomic<Revision> revision_;
    mutable internal::ActivityMap unspent_;
    mutable internal::ActivityMap spent_;

    using SerializedActivity =
        google::protobuf::RepeatedPtrField<proto::BlockchainActivity>;
    using SerializedType = proto::BlockchainAccountData;

    static auto convert(Activity&& in) noexcept -> proto::BlockchainActivity;
    static auto convert(const proto::BlockchainActivity& in) noexcept
        -> Activity;
    static auto convert(const SerializedActivity& in) noexcept
        -> std::vector<Activity>;
    static auto convert(const std::vector<Activity>& in) noexcept
        -> internal::ActivityMap;

    virtual auto account_already_exists(const rLock& lock) const noexcept
        -> bool = 0;
    void process_spent(
        const rLock& lock,
        const Coin& coin,
        const blockchain::Key key,
        const Amount value) const noexcept;
    void process_unspent(
        const rLock& lock,
        const Coin& coin,
        const blockchain::Key key,
        const Amount value) const noexcept;
    virtual auto save(const rLock& lock) const noexcept -> bool = 0;
    auto serialize_common(const rLock& lock, SerializedType& out) const noexcept
        -> void;

    // NOTE call only from final constructor bodies
    auto init() noexcept -> void;
    virtual auto mutable_element(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept(false)
        -> internal::BalanceElement& = 0;

    BalanceNode(
        const api::internal::Core& api,
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        OTIdentifier&& id,
        Identifier& out) noexcept;
    BalanceNode(
        const api::internal::Core& api,
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        const SerializedType& serialized,
        Identifier& out) noexcept(false);

private:
    static constexpr auto ActivityVersion = VersionNumber{1};
    static constexpr auto BlockchainAccountDataVersion = VersionNumber{1};

    virtual auto check_activity(
        const rLock& lock,
        const std::vector<Activity>& unspent,
        std::set<OTIdentifier>& contacts,
        const PasswordPrompt& reason) const noexcept -> bool = 0;

    virtual auto confirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void
    {
    }
    virtual auto unconfirm(
        const rLock& lock,
        const Subchain type,
        const Bip32Index index) noexcept -> void
    {
    }

    BalanceNode(
        const api::internal::Core& api,
        const internal::BalanceTree& parent,
        const BalanceNodeType type,
        OTIdentifier&& id,
        const Revision revision,
        const std::vector<Activity>& unspent,
        const std::vector<Activity>& spent,
        Identifier& out) noexcept;
    BalanceNode() = delete;
    BalanceNode(const BalanceNode&) = delete;
    BalanceNode(BalanceNode&&) = delete;
    auto operator=(const BalanceNode&) -> BalanceNode& = delete;
    auto operator=(BalanceNode&&) -> BalanceNode& = delete;
};
}  // namespace opentxs::api::client::blockchain::implementation
