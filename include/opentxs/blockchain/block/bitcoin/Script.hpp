// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_SCRIPT_HPP
#define OPENTXS_BLOCKCHAIN_BLOCK_BITCOIN_SCRIPT_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <optional>
#include <tuple>

#include "opentxs/Bytes.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/iterator/Bidirectional.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Blockchain;
class Manager;
}  // namespace client

class Core;
}  // namespace api

class PaymentCode;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Script
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
        Input = 253,
        Empty = 254,
        Malformed = 255,
    };

    enum class Position : std::uint8_t {
        Coinbase = 0,
        Input = 1,
        Output = 2,
    };

    OPENTXS_EXPORT virtual auto at(const std::size_t position) const
        noexcept(false) -> const value_type& = 0;
    OPENTXS_EXPORT virtual auto begin() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto CalculateHash160(
        const api::Core& api,
        const AllocateOutput output) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto CalculateSize() const noexcept
        -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto cbegin() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto cend() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto end() const noexcept -> const_iterator = 0;
    OPENTXS_EXPORT virtual auto ExtractElements(
        const filter::Type style) const noexcept -> std::vector<Space> = 0;
    OPENTXS_EXPORT virtual auto ExtractPatterns(
        const api::Core& api,
        const api::client::Blockchain& blockchain) const noexcept
        -> std::vector<PatternID> = 0;
    OPENTXS_EXPORT virtual auto IsNotification(
        const std::uint8_t version,
        const PaymentCode& recipient) const noexcept -> bool = 0;
    /// Value only present for Multisig patterns
    OPENTXS_EXPORT virtual auto M() const noexcept
        -> std::optional<std::uint8_t> = 0;
    /// Value only present for Multisig patterns, 0 indexed
    OPENTXS_EXPORT virtual auto MultisigPubkey(const std::size_t position)
        const noexcept -> std::optional<ReadView> = 0;
    /// Value only present for Multisig patterns
    OPENTXS_EXPORT virtual auto N() const noexcept
        -> std::optional<std::uint8_t> = 0;
    /// Value only present for PayToPubkey patterns
    OPENTXS_EXPORT virtual auto Pubkey() const noexcept
        -> std::optional<ReadView> = 0;
    /// Value only present for PayToPubkeyHash patterns
    OPENTXS_EXPORT virtual auto PubkeyHash() const noexcept
        -> std::optional<ReadView> = 0;
    /// Value only present for input scripts which spend PayToScriptHash outputs
    OPENTXS_EXPORT virtual auto RedeemScript() const noexcept
        -> std::unique_ptr<Script> = 0;
    OPENTXS_EXPORT virtual auto Role() const noexcept -> Position = 0;
    /// Value only present for PayToScriptHash patterns
    OPENTXS_EXPORT virtual auto ScriptHash() const noexcept
        -> std::optional<ReadView> = 0;
    OPENTXS_EXPORT virtual auto Serialize(
        const AllocateOutput destination) const noexcept -> bool = 0;
    OPENTXS_EXPORT virtual auto size() const noexcept -> std::size_t = 0;
    OPENTXS_EXPORT virtual auto Type() const noexcept -> Pattern = 0;
    /// Value only present for NullData patterns, 0 indexed
    OPENTXS_EXPORT virtual auto Value(const std::size_t position) const noexcept
        -> std::optional<ReadView> = 0;

    virtual ~Script() = default;

protected:
    Script() noexcept = default;

private:
    Script(const Script&) = delete;
    Script(Script&&) = delete;
    Script& operator=(const Script&) = delete;
    Script& operator=(Script&&) = delete;
};
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain
}  // namespace opentxs
#endif
