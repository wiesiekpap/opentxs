// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CLIENT_WALLET_HPP
#define OPENTXS_BLOCKCHAIN_CLIENT_WALLET_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <set>
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
}  // namespace bitcoin

struct Outpoint;
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
namespace node
{
class OPENTXS_EXPORT Wallet
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

    using UTXO =
        std::pair<block::Outpoint, std::unique_ptr<block::bitcoin::Output>>;

    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetBalance(
        const identifier::Nym& owner,
        const Identifier& node) const noexcept -> Balance = 0;
    virtual auto GetOutputs(TxoState type = TxoState::All) const noexcept
        -> std::vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        TxoState type = TxoState::All) const noexcept -> std::vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        TxoState type = TxoState::All) const noexcept -> std::vector<UTXO> = 0;

    virtual ~Wallet() = default;

protected:
    Wallet() noexcept = default;

private:
    Wallet(const Wallet&) = delete;
    Wallet(Wallet&&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
#endif
