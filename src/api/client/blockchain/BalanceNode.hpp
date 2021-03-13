// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <atomic>
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
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
    struct Element final : virtual public internal::BalanceElement {
        enum class Availability {
            NeverUsed,
            Reissue,
            StaleUnconfirmed,
            MetadataConflict,
            Reserved,
            Used,
        };

        auto Address(const AddressStyle format) const noexcept
            -> std::string final;
        auto Confirmed() const noexcept -> Txids final;
        auto Contact() const noexcept -> OTIdentifier final;
        auto Elements() const noexcept -> std::set<OTData> final;
        auto elements(const rLock& lock) const noexcept -> std::set<OTData>;
        auto ID() const noexcept -> const Identifier& final
        {
            return parent_.ID();
        }
        auto IncomingTransactions() const noexcept
            -> std::set<std::string> final;
        auto IsAvailable(const Identifier& contact, const std::string& memo)
            const noexcept -> Availability;
        auto Index() const noexcept -> Bip32Index final { return index_; }
        auto Key() const noexcept -> ECKey final;
        auto KeyID() const noexcept -> blockchain::Key final
        {
            return {ID().str(), subchain_, index_};
        }
        auto Label() const noexcept -> std::string final;
        auto LastActivity() const noexcept -> Time final;
        auto NymID() const noexcept -> const identifier::Nym& final
        {
            return parent_.Parent().NymID();
        }
        auto Parent() const noexcept -> const blockchain::BalanceNode& final
        {
            return parent_;
        }
        auto PrivateKey(const PasswordPrompt& reason) const noexcept
            -> ECKey final
        {
            return parent_.PrivateKey(subchain_, index_, reason);
        }
        auto PubkeyHash() const noexcept -> OTData final;
        auto Serialize() const noexcept -> SerializedType final;
        auto Subchain() const noexcept -> blockchain::Subchain final
        {
            return subchain_;
        }
        auto Unconfirmed() const noexcept -> Txids final;

        auto Confirm(const Txid& tx) noexcept -> bool final;
        auto Reserve(const Time time) noexcept -> bool final;
        auto SetContact(const Identifier& id) noexcept -> void final;
        auto SetLabel(const std::string& label) noexcept -> void final;
        auto SetMetadata(
            const Identifier& contact,
            const std::string& label) noexcept -> void final;
        auto Unconfirm(const Txid& tx, const Time time) noexcept -> bool final;
        auto Unreserve() noexcept -> bool final;

        Element(
            const api::internal::Core& api,
            const client::internal::Blockchain& blockchain,
            const internal::BalanceNode& parent,
            const opentxs::blockchain::Type chain,
            const blockchain::Subchain subchain,
            const Bip32Index index,
            const opentxs::crypto::key::EllipticCurve& key) noexcept(false);
        Element(
            const api::internal::Core& api,
            const client::internal::Blockchain& blockchain,
            const internal::BalanceNode& parent,
            const opentxs::blockchain::Type chain,
            const blockchain::Subchain subchain,
            const SerializedType& address) noexcept(false);
        ~Element() final = default;

    private:
        using pTxid = opentxs::blockchain::block::pTxid;
        using Transactions = boost::container::flat_set<pTxid>;

        static const VersionNumber DefaultVersion{1};

        const api::internal::Core& api_;
        const client::internal::Blockchain& blockchain_;
        const internal::BalanceNode& parent_;
        const opentxs::blockchain::Type chain_;
        mutable std::recursive_mutex lock_;
        const VersionNumber version_;
        const blockchain::Subchain subchain_;
        const Bip32Index index_;
        std::string label_;
        OTIdentifier contact_;
        std::shared_ptr<opentxs::crypto::key::EllipticCurve> pkey_;
        opentxs::crypto::key::EllipticCurve& key_;
        Time timestamp_;
        Transactions unconfirmed_;
        Transactions confirmed_;

        static auto instantiate(
            const api::internal::Core& api,
            const proto::AsymmetricKey& serialized) noexcept(false)
            -> std::unique_ptr<opentxs::crypto::key::EllipticCurve>;

        auto update_element(rLock& lock) const noexcept -> void;

        Element(
            const api::internal::Core& api,
            const client::internal::Blockchain& blockchain,
            const internal::BalanceNode& parent,
            const opentxs::blockchain::Type chain,
            const VersionNumber version,
            const blockchain::Subchain subchain,
            const Bip32Index index,
            const std::string label,
            const OTIdentifier contact,
            const opentxs::crypto::key::EllipticCurve& key,
            const Time time,
            Transactions&& unconfirmed,
            Transactions&& confirmed) noexcept(false);
        Element() = delete;
    };

    using AddressMap = std::map<Bip32Index, Element>;
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
