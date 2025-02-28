// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <tuple>

#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Iterator.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
class OPENTXS_EXPORT Script
{
public:
    using value_type = ScriptElement;
    using const_iterator =
        opentxs::iterator::Bidirectional<const Script, const value_type>;

    enum class Pattern : std::uint8_t {
        Custom = 0,
        Coinbase,
        NullData,
        PayToMultisig,
        PayToPubkey,
        PayToPubkeyHash,
        PayToScriptHash,
        PayToWitnessPubkeyHash,
        PayToWitnessScriptHash,
        PayToTaproot,
        None = 252,
        Input = 253,
        Empty = 254,
        Malformed = 255,
    };

    enum class Position : std::uint8_t {
        Coinbase = 0,
        Input = 1,
        Output = 2,
        Redeem = 3,
    };

    virtual auto at(const std::size_t position) const noexcept(false)
        -> const value_type& = 0;
    virtual auto begin() const noexcept -> const_iterator = 0;
    virtual auto CalculateHash160(
        const api::Session& api,
        const AllocateOutput output) const noexcept -> bool = 0;
    virtual auto CalculateSize() const noexcept -> std::size_t = 0;
    virtual auto cbegin() const noexcept -> const_iterator = 0;
    virtual auto cend() const noexcept -> const_iterator = 0;
    virtual auto end() const noexcept -> const_iterator = 0;
    virtual auto ExtractElements(const cfilter::Type style) const noexcept
        -> Vector<Vector<std::byte>> = 0;
    virtual auto ExtractPatterns(const api::Session& api) const noexcept
        -> UnallocatedVector<PatternID> = 0;
    virtual auto IsNotification(
        const std::uint8_t version,
        const PaymentCode& recipient) const noexcept -> bool = 0;
    /// Value only present for Multisig patterns
    virtual auto M() const noexcept -> std::optional<std::uint8_t> = 0;
    /// Value only present for Multisig patterns, 0 indexed
    virtual auto MultisigPubkey(const std::size_t position) const noexcept
        -> std::optional<ReadView> = 0;
    /// Value only present for Multisig patterns
    virtual auto N() const noexcept -> std::optional<std::uint8_t> = 0;
    virtual auto Print() const noexcept -> UnallocatedCString = 0;
    /// Value only present for PayToPubkey and PayToTaproot patterns
    virtual auto Pubkey() const noexcept -> std::optional<ReadView> = 0;
    /// Value only present for PayToPubkeyHash and PayToWitnessPubkeyHash
    /// patterns
    virtual auto PubkeyHash() const noexcept -> std::optional<ReadView> = 0;
    /// Value only present for input scripts which spend PayToScriptHash outputs
    virtual auto RedeemScript() const noexcept -> std::unique_ptr<Script> = 0;
    virtual auto Role() const noexcept -> Position = 0;
    /// Value only present for PayToScriptHash and PayToWitnessScriptHash
    /// patterns
    virtual auto ScriptHash() const noexcept -> std::optional<ReadView> = 0;
    virtual auto Serialize(const AllocateOutput destination) const noexcept
        -> bool = 0;
    virtual auto size() const noexcept -> std::size_t = 0;
    virtual auto Type() const noexcept -> Pattern = 0;
    /// Value only present for NullData patterns, 0 indexed
    virtual auto Value(const std::size_t position) const noexcept
        -> std::optional<ReadView> = 0;
    virtual bool CompareScriptElements(const Script& other) const = 0;

    Script(const Script&) = delete;
    Script(Script&&) = delete;
    auto operator=(const Script&) -> Script& = delete;
    auto operator=(Script&&) -> Script& = delete;

    virtual ~Script() = default;

protected:
    Script() noexcept = default;
};
}  // namespace opentxs::blockchain::bitcoin::block
