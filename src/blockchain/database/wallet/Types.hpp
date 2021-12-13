// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "blockchain/database/wallet/Pattern.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/crypto/HashType.hpp"

namespace opentxs
{
auto key_to_bytes(const blockchain::crypto::Key& in) noexcept -> Space;
}  // namespace opentxs

namespace std
{
using Pattern = opentxs::blockchain::database::wallet::db::Pattern;
using Outpoint = opentxs::blockchain::block::Outpoint;
using Position = opentxs::blockchain::block::Position;
using Identifier = opentxs::OTIdentifier;
using Data = opentxs::OTData;
using NymID = opentxs::OTNymID;
using Key = opentxs::blockchain::crypto::Key;

template <>
struct hash<Pattern> {
    auto operator()(const Pattern& data) const noexcept -> std::size_t
    {
        static const auto& api = opentxs::Context().Crypto().Hash();
        static const auto key = std::array<char, 16>{};
        auto out = std::size_t{};
        api.HMAC(
            opentxs::crypto::HashType::SipHash24,
            {key.data(), key.size()},
            opentxs::reader(data.data_),
            opentxs::preallocated(sizeof(out), &out));

        return out;
    }
};

template <>
struct hash<Outpoint> {
    auto operator()(const Outpoint& data) const noexcept -> std::size_t
    {
        static const auto& api = opentxs::Context().Crypto().Hash();
        static const auto key = std::array<char, 16>{};
        auto out = std::size_t{};
        const auto bytes = data.Bytes();
        api.HMAC(
            opentxs::crypto::HashType::SipHash24,
            {key.data(), key.size()},
            {std::next(bytes.data(), 24), 12u},
            opentxs::preallocated(sizeof(out), &out));

        return out;
    }
};

template <>
struct hash<Position> {
    auto operator()(const Position& data) const noexcept -> std::size_t
    {
        auto out = std::size_t{};
        const auto& hash = data.second.get();
        std::memcpy(&out, hash.data(), std::min(sizeof(out), hash.size()));

        return out;
    }
};

template <>
struct hash<Identifier> {
    auto operator()(const opentxs::Identifier& data) const noexcept
        -> std::size_t
    {
        auto out = std::size_t{};
        std::memcpy(&out, data.data(), std::min(sizeof(out), data.size()));

        return out;
    }
};

template <>
struct hash<NymID> {
    auto operator()(const NymID& data) const noexcept -> std::size_t
    {
        static const auto hasher = hash<Identifier>{};

        return hasher(data.get());
    }
};

template <>
struct hash<Data> {
    auto operator()(const Data& data) const noexcept -> std::size_t
    {
        static const auto& api = opentxs::Context().Crypto().Hash();
        static const auto key = std::array<char, 16>{};
        auto out = std::size_t{};
        api.HMAC(
            opentxs::crypto::HashType::SipHash24,
            {key.data(), key.size()},
            data->Bytes(),
            opentxs::preallocated(sizeof(out), &out));

        return out;
    }
};

template <>
struct hash<Key> {
    auto operator()(const Key& data) const noexcept -> std::size_t
    {
        const auto preimage = opentxs::key_to_bytes(data);
        static const auto& api = opentxs::Context().Crypto().Hash();
        static const auto key = std::array<char, 16>{};
        auto out = std::size_t{};
        api.HMAC(
            opentxs::crypto::HashType::SipHash24,
            {key.data(), key.size()},
            opentxs::reader(preimage),
            opentxs::preallocated(sizeof(out), &out));

        return out;
    }
};
}  // namespace std

namespace opentxs::blockchain::database::wallet
{
using Parent = node::internal::WalletDatabase;
using SubchainIndex = Parent::SubchainIndex;
using pSubchainIndex = Parent::pSubchainIndex;
using NodeID = Parent::NodeID;
using pNodeID = Parent::pNodeID;
using Subchain = Parent::Subchain;
using Patterns = Parent::Patterns;
using ElementMap = Parent::ElementMap;
using MatchingIndices = Parent::MatchingIndices;
using PatternID = Identifier;
using pPatternID = OTIdentifier;
}  // namespace opentxs::blockchain::database::wallet
