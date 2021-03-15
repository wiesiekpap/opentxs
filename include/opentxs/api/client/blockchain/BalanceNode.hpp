// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODE_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODE_HPP

// IWYU pragma: no_include "opentxs/Proto.hpp"

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/blockchain/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace blockchain
{
class BalanceNode
{
public:
    using Txid = opentxs::blockchain::block::Txid;
    using Txids = std::vector<opentxs::blockchain::block::pTxid>;

    struct Element {
        OPENTXS_EXPORT virtual std::string Address(
            const AddressStyle format) const noexcept = 0;
        OPENTXS_EXPORT virtual Txids Confirmed() const noexcept = 0;
        OPENTXS_EXPORT virtual OTIdentifier Contact() const noexcept = 0;
        OPENTXS_EXPORT virtual Bip32Index Index() const noexcept = 0;
        OPENTXS_EXPORT virtual ECKey Key() const noexcept = 0;
        OPENTXS_EXPORT virtual blockchain::Key KeyID() const noexcept = 0;
        OPENTXS_EXPORT virtual std::string Label() const noexcept = 0;
        OPENTXS_EXPORT virtual Time LastActivity() const noexcept = 0;
        OPENTXS_EXPORT virtual const BalanceNode& Parent() const noexcept = 0;
        OPENTXS_EXPORT virtual ECKey PrivateKey(
            const PasswordPrompt& reason) const noexcept = 0;
        OPENTXS_EXPORT virtual OTData PubkeyHash() const noexcept = 0;
        OPENTXS_EXPORT virtual blockchain::Subchain Subchain()
            const noexcept = 0;
        OPENTXS_EXPORT virtual Txids Unconfirmed() const noexcept = 0;

        virtual ~Element() = default;

    protected:
        Element() noexcept = default;
    };

    /// Throws std::out_of_range for invalid index
    OPENTXS_EXPORT virtual const Element& BalanceElement(
        const Subchain type,
        const Bip32Index index) const noexcept(false) = 0;
    OPENTXS_EXPORT virtual const Identifier& ID() const noexcept = 0;
    OPENTXS_EXPORT virtual const BalanceTree& Parent() const noexcept = 0;
    OPENTXS_EXPORT virtual BalanceNodeType Type() const noexcept = 0;

    OPENTXS_EXPORT virtual ~BalanceNode() = default;

protected:
    BalanceNode() noexcept = default;

private:
    BalanceNode(const BalanceNode&) = delete;
    BalanceNode(BalanceNode&&) = delete;
    BalanceNode& operator=(const BalanceNode&) = delete;
    BalanceNode& operator=(BalanceNode&&) = delete;
};
}  // namespace blockchain
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif  // OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODE_HPP
