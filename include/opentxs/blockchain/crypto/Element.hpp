// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_CRYPTO_ELEMENT_HPP
#define OPENTXS_BLOCKCHAIN_CRYPTO_ELEMENT_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class Subaccount;
}  // namespace crypto
}  // namespace blockchain

class PasswordPrompt;
}  // namespace opentxs

namespace opentxs
{
namespace blockchain
{
namespace crypto
{
class OPENTXS_EXPORT Element
{
public:
    using Txids = std::vector<opentxs::blockchain::block::pTxid>;

    virtual std::string Address(const AddressStyle format) const noexcept = 0;
    virtual Txids Confirmed() const noexcept = 0;
    virtual OTIdentifier Contact() const noexcept = 0;
    virtual Bip32Index Index() const noexcept = 0;
    virtual ECKey Key() const noexcept = 0;
    virtual crypto::Key KeyID() const noexcept = 0;
    virtual std::string Label() const noexcept = 0;
    virtual Time LastActivity() const noexcept = 0;
    virtual const Subaccount& Parent() const noexcept = 0;
    virtual ECKey PrivateKey(const PasswordPrompt& reason) const noexcept = 0;
    virtual OTData PubkeyHash() const noexcept = 0;
    virtual crypto::Subchain Subchain() const noexcept = 0;
    virtual Txids Unconfirmed() const noexcept = 0;

    OPENTXS_NO_EXPORT virtual ~Element() = default;

protected:
    Element() noexcept = default;

private:
    Element(const Element&) = delete;
    Element(Element&&) = delete;
    Element& operator=(const Element&) = delete;
    Element& operator=(Element&&) = delete;
};
}  // namespace crypto
}  // namespace blockchain
}  // namespace opentxs
#endif  // OPENTXS_BLOCKCHAIN_CRYPTO_BALANCENODE_HPP
