// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "internal/blockchain/crypto/Crypto.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/intrusive/detail/iterator.hpp>
#include <boost/move/algo/detail/set_difference.hpp>
#include <boost/move/algo/move.hpp>
#include <memory>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/HashType.hpp"

namespace opentxs
{
auto blockchain_thread_item_id(
    const api::Crypto& crypto,
    const opentxs::blockchain::Type chain,
    const Data& txid) noexcept -> OTIdentifier
{
    auto preimage = std::string{};
    const auto hashed = crypto.Hash().HMAC(
        crypto::HashType::Sha256,
        ReadView{reinterpret_cast<const char*>(&chain), sizeof(chain)},
        txid.Bytes(),
        writer(preimage));

    OT_ASSERT(hashed);

    auto id = Identifier::Factory();
    id->CalculateDigest(preimage);

    return id;
}

auto operator==(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return false; }

    if (lSubchain != rSubchain) { return false; }

    return lIndex == rIndex;
}

auto operator!=(
    const blockchain::crypto::Key& lhs,
    const blockchain::crypto::Key& rhs) noexcept -> bool
{
    const auto& [lAccount, lSubchain, lIndex] = lhs;
    const auto& [rAccount, rSubchain, rIndex] = rhs;

    if (lAccount != rAccount) { return true; }

    if (lSubchain != rSubchain) { return true; }

    return lIndex != rIndex;
}

auto print(blockchain::crypto::HDProtocol value) noexcept -> std::string
{
    using Proto = blockchain::crypto::HDProtocol;
    static const auto map = boost::container::flat_map<Proto, std::string>{
        {Proto::BIP_32, "BIP-32"},
        {Proto::BIP_44, "BIP-44"},
        {Proto::BIP_49, "BIP-49"},
        {Proto::BIP_84, "BIP-84"},
        {Proto::Error, "invalid"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return map.at(Proto::Error);
    }
}

auto print(blockchain::crypto::Subchain value) noexcept -> std::string
{
    using Subchain = blockchain::crypto::Subchain;
    static const auto map = boost::container::flat_map<Subchain, std::string>{
        {Subchain::Internal, "internal"},
        {Subchain::External, "external"},
        {Subchain::Incoming, "incoming"},
        {Subchain::Outgoing, "outgoing"},
        {Subchain::Notification, "notification"},
        {Subchain::None, "none"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}

auto print(const blockchain::crypto::Key& key) noexcept -> std::string
{
    const auto& [account, subchain, index] = key;

    return account + " / " + print(subchain) + " / " + std::to_string(index);
}
}  // namespace opentxs
