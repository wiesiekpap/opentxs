// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/container/flat_set.hpp>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
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
    auto Keys() const noexcept -> std::vector<KeyID> final;
    auto NetBalanceChange(
        const api::client::Blockchain& blockchain,
        const identifier::Nym& nym) const noexcept -> opentxs::Amount final;
    auto PreviousOutput() const noexcept -> const Outpoint& final
    {
        return previous_;
    }
    auto PrintScript() const noexcept -> std::string final
    {
        return script_->str();
    }
    auto Serialize(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto SerializeNormalized(const AllocateOutput destination) const noexcept
        -> std::optional<std::size_t> final;
    auto Serialize(
        const api::client::Blockchain& blockchain,
        const std::uint32_t index,
        SerializeType& destination) const noexcept -> bool final;
    auto SignatureVersion() const noexcept
        -> std::unique_ptr<internal::Input> final;
    auto SignatureVersion(std::unique_ptr<internal::Script> subscript)
        const noexcept -> std::unique_ptr<internal::Input> final;
    auto Script() const noexcept -> const bitcoin::Script& final
    {
        return *script_;
    }
    auto Sequence() const noexcept -> std::uint32_t final { return sequence_; }
    auto Spends() const noexcept(false) -> const internal::Output& final;
    auto Witness() const noexcept -> const std::vector<Space>& final
    {
        return witness_;
    }

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
    static const VersionNumber outpoint_version_;
    static const VersionNumber key_version_;

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
    mutable std::unique_ptr<const internal::Output> previous_output_;
    mutable std::optional<std::size_t> size_;
    mutable std::optional<std::size_t> normalized_size_;
    mutable boost::container::flat_set<KeyID> keys_;

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
