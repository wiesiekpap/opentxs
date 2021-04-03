// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "api/client/blockchain/database/BlockHeaders.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

#include "opentxs/Proto.tpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/protobuf/BlockchainBlockHeader.pb.h"
#include "util/LMDB.hpp"

// #define OT_METHOD
// "opentxs::api::client::blockchain::database::implementation::BlockHeader::"

namespace opentxs::api::client::blockchain::database::implementation
{
BlockHeader::BlockHeader(
    const api::Core& api,
    opentxs::storage::lmdb::LMDB& lmdb) noexcept(false)
    : api_(api)
    , lmdb_(lmdb)
{
}

auto BlockHeader::BlockHeaderExists(
    const opentxs::blockchain::block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Exists(
        Table::BlockHeaders, api_.Crypto().Encode().IdentifierEncode(hash));
}

auto BlockHeader::LoadBlockHeader(const opentxs::blockchain::block::Hash& hash)
    const noexcept(false) -> proto::BlockchainBlockHeader
{
    auto output = std::optional<proto::BlockchainBlockHeader>{};
    lmdb_.Load(
        Table::BlockHeaders,
        api_.Crypto().Encode().IdentifierEncode(hash),
        [&](const auto data) -> void {
            output = proto::Factory<proto::BlockchainBlockHeader>(
                data.data(), data.size());
        });

    if (false == output.has_value()) {
        throw std::out_of_range("Block header not found");
    }

    return output.value();
}

auto BlockHeader::StoreBlockHeader(
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    auto serialized = header.Serialize();
    serialized.clear_local();
    const auto result = lmdb_.Store(
        Table::BlockHeaders,
        api_.Crypto().Encode().IdentifierEncode(header.Hash()),
        proto::ToString(header.Serialize()),
        nullptr,
        MDB_NOOVERWRITE);

    if (false == result.first) {
        if (MDB_KEYEXIST != result.second) { return false; }
    }

    return true;
}

auto BlockHeader::StoreBlockHeaders(const UpdatedHeader& headers) const noexcept
    -> bool
{
    auto parentTxn = lmdb_.TransactionRW();

    for (const auto& [hash, pair] : headers) {
        const auto& [header, newBlock] = pair;

        if (newBlock) {
            auto serialized = header->Serialize();
            serialized.clear_local();
            const auto stored = lmdb_.Store(
                Table::BlockHeaders,
                api_.Crypto().Encode().IdentifierEncode(header->Hash()),
                proto::ToString(serialized),
                parentTxn,
                MDB_NOOVERWRITE);

            if (false == stored.first) {
                if (MDB_KEYEXIST != stored.second) { return false; }
            }
        }
    }

    return parentTxn.Finalize(true);
}
}  // namespace opentxs::api::client::blockchain::database::implementation
