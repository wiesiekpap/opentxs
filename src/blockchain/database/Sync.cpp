// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/database/Sync.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "util/LMDB.hpp"

#define OT_METHOD "opentxs::blockchain::database::Sync::"

namespace opentxs::blockchain::database
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

Sync::Sync(
    const api::Core& api,
    const common::Database& common,
    const storage::lmdb::LMDB& lmdb,
    const blockchain::Type type) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_(make_blank<block::Position>::value(api))
    , chain_(type)
    , genesis_([&] {
        const auto& hex = params::Data::Chains().at(chain_).genesis_hash_hex_;

        return api_.Factory().Data(hex, StringStyle::Hex);
    }())
{
    auto tip = Tip();

    if (blank_position_.first == tip.first) {
        const auto genesis = block::Position{0, genesis_};
        const auto saved = SetTip(genesis);
        tip = genesis;

        OT_ASSERT(saved);
    }

    LogVerbose(OT_METHOD)(__func__)(": Sync tip: ")(tip.first).Flush();

    if (const auto ctip = common_.SyncTip(chain_); tip.first == ctip) {
        LogVerbose(OT_METHOD)(__func__)(": Database is consistent").Flush();
    } else {
        LogVerbose(OT_METHOD)(__func__)(
            ": Database inconsistency detected. Storage tip height: ")(ctip)
            .Flush();
    }
}

auto Sync::Load(const block::Height height, Message& output) const noexcept
    -> bool
{
    return common_.LoadSync(chain_, height, output);
}

auto Sync::Reorg(const block::Height height) const noexcept -> bool
{
    return common_.ReorgSync(chain_, height);
}

auto Sync::SetTip(const block::Position& position) const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::Config,
            tsv(static_cast<std::size_t>(Key::SyncPosition)),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Sync::Store(const block::Position& tip, const Items& items) const noexcept
    -> bool
{
    if (false == common_.StoreSync(chain_, items)) {
        LogOutput(OT_METHOD)(__func__)(": Failed to store sync data").Flush();

        return false;
    }

    return SetTip(tip);
}

auto Sync::Tip() const noexcept -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(
        Table::Config, tsv(static_cast<std::size_t>(Key::SyncPosition)), cb);

    return output;
}
}  // namespace opentxs::blockchain::database
