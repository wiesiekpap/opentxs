// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/intrusive/detail/iterator.hpp>

#pragma once

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
}  // namespace client

class Core;
}  // namespace api

class Core;
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Output final : public internal::Output
{
public:
    auto AssociatedLocalNyms(
        const api::client::Blockchain& blockchain,
        std::vector<OTNymID>& output) const noexcept -> void final;
    auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        std::vector<OTIdentifier>& output) const noexcept -> void final;
    auto CalculateSize() const noexcept -> std::size_t final;
    auto clone() const noexcept -> std::unique_ptr<internal::Output> final
    {
        return std::make_unique<Output>(*this);
    }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const ParsedPatterns& patterns) const noexcept -> Matches final;
    auto GetPatterns() const noexcept -> std::vector<PatternID> final;
    auto Keys() const noexcept -> std::vector<KeyID> final
    {
        return cache_.keys();
    }
    auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount final;
    auto Note(const api::client::Blockchain& blockchain) const noexcept
        -> std::string final;
    auto Payee() const noexcept -> ContactID final { return cache_.payee(); }
    auto Payer() const noexcept -> ContactID final { return cache_.payer(); }
    auto Print() const noexcept -> std::string final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(
        const api::client::Blockchain& blockchain,
        SerializeType& destination) const noexcept -> bool final;
    auto SigningSubscript() const noexcept
        -> std::unique_ptr<internal::Script> final
    {
        return script_->SigningSubscript(chain_);
    }
    auto Script() const noexcept -> const internal::Script& final
    {
        return *script_;
    }
    auto Value() const noexcept -> std::int64_t final { return value_; }

    auto ForTestingOnlyAddKey(const KeyID& key) noexcept -> void final
    {
        cache_.add(KeyID{key});
    }
    auto MergeMetadata(const SerializeType& rhs) noexcept -> void final;
    auto SetIndex(const std::uint32_t index) noexcept -> void final
    {
        const_cast<std::uint32_t&>(index_) = index;
    }
    auto SetKeyData(const KeyData& data) noexcept -> void final
    {
        cache_.set(data);
    }
    auto SetPayee(const Identifier& contact) noexcept -> void final
    {
        cache_.set_payee(contact);
    }
    auto SetPayer(const Identifier& contact) noexcept -> void final
    {
        cache_.set_payer(contact);
    }
    auto SetValue(const std::uint64_t value) noexcept -> void final
    {
        const_cast<std::uint64_t&>(value_) = value;
    }

    Output(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const std::uint32_t index,
        const std::uint64_t value,
        const std::size_t size,
        const ReadView script,
        const VersionNumber version = default_version_) noexcept(false);
    Output(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const std::uint32_t index,
        const std::uint64_t value,
        std::unique_ptr<const internal::Script> script,
        boost::container::flat_set<KeyID>&& keys,
        const VersionNumber version = default_version_) noexcept(false);
    Output(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const VersionNumber version,
        const std::uint32_t index,
        const std::uint64_t value,
        std::unique_ptr<const internal::Script> script,
        std::optional<std::size_t> size,
        boost::container::flat_set<KeyID>&& keys,
        boost::container::flat_set<PatternID>&& pubkeyHashes,
        std::optional<PatternID>&& scriptHash,
        const bool indexed) noexcept(false);
    Output(const Output&) noexcept;

    ~Output() final = default;

private:
    struct Cache {
        template <typename F>
        auto for_each_key(F cb) const noexcept -> void
        {
            auto lock = Lock{lock_};
            std::for_each(std::begin(keys_), std::end(keys_), cb);
        }
        auto keys() const noexcept -> std::vector<KeyID>;
        auto payee() const noexcept -> OTIdentifier;
        auto payer() const noexcept -> OTIdentifier;

        auto add(KeyID&& key) noexcept -> void;
        auto reset_size() noexcept -> void;
        auto set(const KeyData& data) noexcept -> void;
        auto set_payee(const Identifier& contact) noexcept -> void;
        auto set_payer(const Identifier& contact) noexcept -> void;
        template <typename F>
        auto size(F cb) noexcept -> std::size_t
        {
            auto lock = Lock{lock_};

            auto& output = size_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }

        Cache(
            const api::Core& api,
            std::optional<std::size_t>&& size,
            boost::container::flat_set<KeyID>&& keys) noexcept;
        Cache(const Cache& rhs) noexcept;

    private:
        mutable std::mutex lock_{};
        std::optional<std::size_t> size_{};
        OTIdentifier payee_;
        OTIdentifier payer_;
        boost::container::flat_set<KeyID> keys_;

        Cache() noexcept = delete;
    };

    static const VersionNumber default_version_;
    static const VersionNumber key_version_;

    const api::Core& api_;
    const api::client::Blockchain& crypto_;
    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const std::uint32_t index_;
    const std::uint64_t value_;
    const std::unique_ptr<const internal::Script> script_;
    const boost::container::flat_set<PatternID> pubkey_hashes_;
    const std::optional<PatternID> script_hash_;
    mutable Cache cache_;

    auto index_elements() noexcept -> void;

    Output() = delete;
    Output(Output&&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
