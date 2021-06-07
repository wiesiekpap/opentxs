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
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/protobuf/BlockchainTransactionInput.pb.h"
#include "opentxs/protobuf/BlockchainTransactionOutput.pb.h"
#include "opentxs/protobuf/BlockchainWalletKey.pb.h"

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

namespace proto
{
class BlockchainTransactionInput;
class BlockchainTransactionOutput;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::blockchain::block::bitcoin::implementation
{
class Input final : public internal::Input
{
public:
    static const VersionNumber default_version_;

    auto AssociatedLocalNyms(
        const api::client::Blockchain& blockchain,
        std::vector<OTNymID>& output) const noexcept -> void final;
    auto AssociatedRemoteContacts(
        const api::client::Blockchain& blockchain,
        std::vector<OTIdentifier>& output) const noexcept -> void final;
    auto CalculateSize(const bool normalized) const noexcept
        -> std::size_t final;
    auto Coinbase() const noexcept -> Space final { return coinbase_; }
    auto clone() const noexcept -> std::unique_ptr<internal::Input> final
    {
        return std::make_unique<Input>(*this);
    }
    auto ExtractElements(const filter::Type style) const noexcept
        -> std::vector<Space> final;
    auto FindMatches(
        const ReadView txid,
        const FilterType type,
        const Patterns& txos,
        const ParsedPatterns& elements) const noexcept -> Matches final;
    auto GetPatterns() const noexcept -> std::vector<PatternID> final;
    auto Keys() const noexcept -> std::vector<KeyID> final
    {
        return cache_.keys();
    }
    auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount final
    {
        return cache_.net_balance_change(blockchain, nym);
    }
    auto Payer() const noexcept -> OTIdentifier { return cache_.payer(); }
    auto PreviousOutput() const noexcept -> const Outpoint& final
    {
        return previous_;
    }
    auto Print() const noexcept -> std::string final;
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto SerializeNormalized(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(
        const api::client::Blockchain& blockchain,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool final;
    auto SetKeyData(const KeyData& data) noexcept -> void final
    {
        return cache_.set(data);
    }
    auto SignatureVersion() const noexcept
        -> std::unique_ptr<internal::Input> final;
    auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<internal::Input> final;
    auto Script() const noexcept -> const bitcoin::Script& final
    {
        return *script_;
    }
    auto Sequence() const noexcept -> std::uint32_t final { return sequence_; }
    auto Spends() const noexcept(false) -> const internal::Output& final
    {
        return cache_.spends();
    }
    auto Witness() const noexcept -> const std::vector<Space>& final
    {
        return witness_;
    }

    auto AddMultisigSignatures(const Signatures& signatures) noexcept
        -> bool final;
    auto AddSignatures(const Signatures& signatures) noexcept -> bool final;
    auto AssociatePreviousOutput(
        const api::client::Blockchain& blockchain,
        const proto::BlockchainTransactionOutput& output) noexcept
        -> bool final;
    auto MergeMetadata(
        const api::client::Blockchain& blockchain,
        const SerializeType& rhs) noexcept -> void final;
    auto ReplaceScript() noexcept -> bool final;

    Input(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        std::unique_ptr<const internal::Script> script,
        const VersionNumber version,
        std::optional<std::size_t> size) noexcept(false);
    Input(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        std::unique_ptr<const internal::Script> script,
        const VersionNumber version,
        std::unique_ptr<const internal::Output> output,
        boost::container::flat_set<KeyID>&& keys) noexcept(false);
    Input(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        const ReadView coinbase,
        const VersionNumber version,
        std::unique_ptr<const internal::Output> output,
        std::optional<std::size_t> size = {}) noexcept(false);
    Input(
        const api::Core& api,
        const api::client::Blockchain& blockchain,
        const blockchain::Type chain,
        const std::uint32_t sequence,
        Outpoint&& previous,
        std::vector<Space>&& witness,
        std::unique_ptr<const internal::Script> script,
        Space&& coinbase,
        const VersionNumber version,
        std::optional<std::size_t> size,
        boost::container::flat_set<KeyID>&& keys,
        boost::container::flat_set<PatternID>&& pubkeyHashes,
        std::optional<PatternID>&& scriptHash,
        const bool indexed,
        std::unique_ptr<const internal::Output> output) noexcept(false);
    Input(const Input&) noexcept;
    Input(
        const Input& rhs,
        std::unique_ptr<const internal::Script> script) noexcept;

    ~Input() final = default;

private:
    struct Cache {
        template <typename F>
        auto for_each_key(F cb) const noexcept -> void
        {
            auto lock = rLock{lock_};
            std::for_each(std::begin(keys_), std::end(keys_), cb);
        }
        auto keys() const noexcept -> std::vector<KeyID>
        {
            auto lock = rLock{lock_};
            auto output = std::vector<KeyID>{};
            std::transform(
                std::begin(keys_),
                std::end(keys_),
                std::back_inserter(output),
                [](const auto& key) -> auto { return key; });

            return output;
        }
        auto net_balance_change(
            const api::client::Blockchain& blockchain,
            const identifier::Nym& nym) const noexcept -> opentxs::Amount
        {
            auto lock = rLock{lock_};

            if (false == bool(previous_output_)) { return 0; }

            for (const auto& key : keys_) {
                if (blockchain.Owner(key) == nym) {
                    return -1 * previous_output_->Value();
                }
            }

            return 0;
        }
        auto payer() const noexcept -> OTIdentifier
        {
            auto lock = rLock{lock_};

            return payer_;
        }
        auto spends() const noexcept(false) -> const internal::Output&
        {
            auto lock = rLock{lock_};

            if (previous_output_) {

                return *previous_output_;
            } else {

                throw std::runtime_error("previous output missing");
            }
        }

        auto add(KeyID&& key) noexcept -> void
        {
            auto lock = rLock{lock_};
            keys_.emplace(std::move(key));
        }
        template <typename F>
        auto associate(
            const api::client::Blockchain& blockchain,
            const proto::BlockchainTransactionOutput& in,
            F cb) noexcept -> bool
        {
            auto lock = rLock{lock_};

            if (false == bool(previous_output_)) { previous_output_ = cb(); }

            // NOTE this should only happen during unit testing
            if (keys_.empty()) {
                OT_ASSERT(0 < in.key_size());

                for (const auto& key : in.key()) {
                    keys_.emplace(
                        key.subaccount(),
                        static_cast<blockchain::crypto::Subchain>(
                            static_cast<std::uint8_t>(key.subchain())),
                        key.index());
                }
            }

            return bool(previous_output_);
        }
        template <typename F>
        auto merge(
            const api::client::Blockchain& blockchain,
            const SerializeType& rhs,
            F cb) noexcept -> void
        {
            auto lock = rLock{lock_};
            std::for_each(
                std::begin(rhs.key()),
                std::end(rhs.key()),
                [this](const auto& key) {
                    keys_.emplace(
                        key.subaccount(),
                        static_cast<blockchain::crypto::Subchain>(
                            static_cast<std::uint8_t>(key.subchain())),
                        key.index());
                });

            if (rhs.has_spends() && (false == bool(previous_output_))) {
                previous_output_ = cb();
            }
        }
        auto reset_size() noexcept -> void
        {
            auto lock = rLock{lock_};
            size_ = std::nullopt;
            normalized_size_ = std::nullopt;
        }
        auto set(const KeyData& data) noexcept -> void
        {
            auto lock = rLock{lock_};

            if (payer_->empty()) {
                for (const auto& key : keys_) {
                    try {
                        const auto& [sender, recipient] = data.at(key);

                        if (recipient->empty()) { continue; }

                        payer_ = recipient;

                        return;
                    } catch (...) {
                    }
                }
            }
        }
        template <typename F>
        auto size(const bool normalize, F cb) noexcept -> std::size_t
        {
            auto lock = rLock{lock_};
            auto& output = normalize ? normalized_size_ : size_;

            if (false == output.has_value()) { output = cb(); }

            return output.value();
        }

        Cache(
            const api::Core& api,
            std::unique_ptr<const internal::Output>&& output,
            std::optional<std::size_t>&& size,
            boost::container::flat_set<KeyID>&& keys) noexcept
            : lock_()
            , previous_output_(std::move(output))
            , size_(std::move(size))
            , normalized_size_()
            , keys_(std::move(keys))
            , payer_(api.Factory().Identifier())
        {
        }
        Cache(const Cache& rhs) noexcept
            : lock_()
            , previous_output_()
            , size_()
            , normalized_size_()
            , keys_()
            , payer_([&] {
                auto lock = rLock{rhs.lock_};

                return rhs.payer_;
            }())
        {
            auto lock = rLock{rhs.lock_};

            if (rhs.previous_output_) {
                previous_output_ = rhs.previous_output_->clone();
            }

            size_ = rhs.size_;
            normalized_size_ = rhs.normalized_size_;
            keys_ = rhs.keys_;
        }

    private:
        mutable std::recursive_mutex lock_{};
        std::unique_ptr<const internal::Output> previous_output_;
        std::optional<std::size_t> size_;
        std::optional<std::size_t> normalized_size_;
        boost::container::flat_set<KeyID> keys_;
        OTIdentifier payer_;

        Cache() = delete;
    };

    static const VersionNumber outpoint_version_;
    static const VersionNumber key_version_;

    enum class Redeem : std::uint8_t {
        None,
        MaybeP2WSH,
        P2SH_P2WSH,
        P2SH_P2WPKH,
    };

    const api::Core& api_;
    const blockchain::Type chain_;
    const VersionNumber serialize_version_;
    const Outpoint previous_;
    const std::vector<Space> witness_;
    const std::unique_ptr<const internal::Script> script_;
    const Space coinbase_;
    const std::uint32_t sequence_;
    const boost::container::flat_set<PatternID> pubkey_hashes_;
    const std::optional<PatternID> script_hash_;
    mutable Cache cache_;

    auto classify() const noexcept -> Redeem;
    auto decode_coinbase() const noexcept -> std::string;
    auto is_bip16() const noexcept;
    auto serialize(const AllocateOutput destination, const bool normalized)
        const noexcept -> std::optional<std::size_t>;

    auto index_elements(const api::client::Blockchain& blockchain) noexcept
        -> void;

    Input() = delete;
    Input(Input&&) = delete;
    auto operator=(const Input&) -> Input& = delete;
    auto operator=(Input&&) -> Input& = delete;
};
}  // namespace opentxs::blockchain::block::bitcoin::implementation
