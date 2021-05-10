// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODE_HPP
#define OPENTXS_API_CLIENT_BLOCKCHAIN_BALANCENODE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

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
class BalanceTree;
}  // namespace blockchain
}  // namespace client
}  // namespace api

class PasswordPrompt;
}  // namespace opentxs

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

    struct OPENTXS_EXPORT Element {
        virtual std::string Address(
            const AddressStyle format) const noexcept = 0;
        virtual Txids Confirmed() const noexcept = 0;
        virtual OTIdentifier Contact() const noexcept = 0;
        virtual Bip32Index Index() const noexcept = 0;
        virtual ECKey Key() const noexcept = 0;
        virtual blockchain::Key KeyID() const noexcept = 0;
        virtual std::string Label() const noexcept = 0;
        virtual Time LastActivity() const noexcept = 0;
        virtual const BalanceNode& Parent() const noexcept = 0;
        virtual ECKey PrivateKey(
            const PasswordPrompt& reason) const noexcept = 0;
        virtual OTData PubkeyHash() const noexcept = 0;
        virtual blockchain::Subchain Subchain() const noexcept = 0;
        virtual Txids Unconfirmed() const noexcept = 0;

        virtual OPENTXS_NO_EXPORT ~Element() = default;

    protected:
        Element() noexcept = default;
    };

    /// Throws std::out_of_range for invalid index
    virtual const Element& BalanceElement(
        const Subchain type,
        const Bip32Index index) const noexcept(false) = 0;
    virtual const Identifier& ID() const noexcept = 0;
    virtual const BalanceTree& Parent() const noexcept = 0;
    virtual BalanceNodeType Type() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual ~BalanceNode() = default;

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
