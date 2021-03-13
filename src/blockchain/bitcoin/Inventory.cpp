// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "blockchain/bitcoin/Inventory.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "util/Container.hpp"

// #define OT_METHOD "opentxs::blockchain::bitcoin::CompactSize::"

namespace opentxs::blockchain::bitcoin
{
const std::size_t Inventory::EncodedSize{sizeof(BitcoinFormat)};

const Inventory::Map Inventory::map_{
    {Type::None, 0},
    {Type::MsgTx, 1},
    {Type::MsgBlock, 2},
    {Type::MsgFilteredBlock, 3},
    {Type::MsgCmpctBlock, 4},
    {Type::MsgWitnessTx, 16777280},
    {Type::MsgWitnessBlock, 8388672},
    {Type::MsgFilteredWitnessBlock, 25165888},
};

const Inventory::ReverseMap Inventory::reverse_map_{reverse_map(map_)};

Inventory::Inventory(const Type type, const Hash& hash) noexcept
    : type_(type)
    , hash_(hash)
{
}

Inventory::Inventory(const void* payload, const std::size_t size) noexcept
    : type_(decode_type(payload, size))
    , hash_(decode_hash(payload, size))
{
}

Inventory::Inventory(const Inventory& rhs) noexcept
    : Inventory(rhs.type_, rhs.hash_)
{
}

Inventory::Inventory(Inventory&& rhs) noexcept
    : type_(std::move(rhs.type_))
    , hash_(std::move(rhs.hash_))
{
}

Inventory::BitcoinFormat::BitcoinFormat(
    const Type type,
    const Hash& hash) noexcept(false)
    : type_(encode_type(type))
    , hash_(encode_hash(hash))
{
}

auto Inventory::decode_hash(
    const void* payload,
    const std::size_t size) noexcept(false) -> OTData
{
    if (EncodedSize != size) { throw std::runtime_error("Invalid payload"); }

    auto* it{static_cast<const std::byte*>(payload)};
    it += sizeof(BitcoinFormat::type_);

    return Data::Factory(it, sizeof(BitcoinFormat::hash_));
}

auto Inventory::decode_type(
    const void* payload,
    const std::size_t size) noexcept(false) -> Inventory::Type
{
    p2p::bitcoin::message::InventoryTypeField type{};

    if (EncodedSize != size) { throw std::runtime_error("Invalid payload"); }

    std::memcpy(&type, payload, sizeof(type));

    return reverse_map_.at(type.value());
}

auto Inventory::DisplayType(const Type type) noexcept -> std::string
{
    if (Type::None == type) {

        return "null";
    } else if (Type::MsgTx == type) {

        return "transaction";
    } else if (Type::MsgBlock == type) {

        return "block";
    } else if (Type::MsgFilteredBlock == type) {

        return "filtered block";
    } else if (Type::MsgCmpctBlock == type) {

        return "compact block";
    } else if (Type::MsgWitnessTx == type) {

        return "segwit transaction";
    } else if (Type::MsgWitnessBlock == type) {

        return "segwit block";
    } else if (Type::MsgFilteredWitnessBlock == type) {

        return "filtered segwit block";
    } else {

        return "unknown";
    }
}

auto Inventory::Encode() const noexcept -> OTData
{
    try {
        BitcoinFormat output{type_, hash_};

        return Data::Factory(&output, sizeof(output));
    } catch (...) {
        auto output = Data::Factory();
        output->SetSize(sizeof(BitcoinFormat));

        return Data::Factory();
    }
}

auto Inventory::encode_hash(const Hash& hash) noexcept(false)
    -> p2p::bitcoin::message::HashField
{
    p2p::bitcoin::message::HashField output{};

    if (sizeof(output) != hash.size()) {
        throw std::runtime_error("Invalid hash");
    }

    std::memcpy(output.data(), hash.data(), output.size());

    return output;
}

auto Inventory::encode_type(const Type type) noexcept(false) -> std::uint32_t
{
    return map_.at(type);
}
}  // namespace opentxs::blockchain::bitcoin
