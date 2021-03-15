// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_WALLET_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_WALLET_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <memory>
#include <tuple>

#include "opentxs/blockchain/Types.hpp"

namespace opentxs
{
namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Output;
struct Outpoint;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Identifier;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace client
{
class Wallet
{
public:
    enum class TxoState : std::uint8_t {
        UnconfirmedNew = 0,
        UnconfirmedSpend = 1,
        ConfirmedNew = 2,
        ConfirmedSpend = 3,
        OrphanedNew = 4,
        OrphanedSpend = 5,
        All = 255,
    };

    using UTXO = std::
        pair<block::bitcoin::Outpoint, std::unique_ptr<block::bitcoin::Output>>;

    OPENTXS_EXPORT virtual auto GetBalance() const noexcept -> Balance = 0;
    OPENTXS_EXPORT virtual auto GetBalance(
        const identifier::Nym& owner) const noexcept -> Balance = 0;
    OPENTXS_EXPORT virtual auto GetBalance(
        const identifier::Nym& owner,
        const Identifier& node) const noexcept -> Balance = 0;
    OPENTXS_EXPORT virtual auto GetOutputs(
        TxoState type = TxoState::All) const noexcept -> std::vector<UTXO> = 0;
    OPENTXS_EXPORT virtual auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type = TxoState::All) const noexcept -> std::vector<UTXO> = 0;
    OPENTXS_EXPORT virtual auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        TxoState type = TxoState::All) const noexcept -> std::vector<UTXO> = 0;

    virtual ~Wallet() = default;

protected:
    Wallet() noexcept = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    Wallet& operator=(const Wallet&) = delete;
    Wallet& operator=(Wallet&&) = delete;
};
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs
#endif
