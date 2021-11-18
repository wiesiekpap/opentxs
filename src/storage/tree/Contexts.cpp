// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "storage/tree/Contexts.hpp"  // IWYU pragma: associated

#include <map>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/protobuf/Check.hpp"
#include "internal/protobuf/verify/Context.hpp"
#include "internal/protobuf/verify/StorageNymList.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/Context.pb.h"
#include "serialization/protobuf/StorageItemHash.pb.h"
#include "serialization/protobuf/StorageNymList.pb.h"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{
Contexts::Contexts(const Driver& storage, const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(2);
    }
}

auto Contexts::Delete(const std::string& id) -> bool { return delete_item(id); }

void Contexts::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        LogError()(OT_PRETTY_CLASS())("Failed to load servers index file")
            .Flush();
        OT_FAIL;
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Contexts::Load(
    const std::string& id,
    std::shared_ptr<proto::Context>& output,
    std::string& alias,
    const bool checking) const -> bool
{
    return load_proto<proto::Context>(id, output, alias, checking);
}

auto Contexts::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        LogError()(OT_PRETTY_CLASS())("Lock failure").Flush();
        OT_FAIL;
    }

    auto serialized = serialize();

    if (false == proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Contexts::serialize() const -> proto::StorageNymList
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_nym());
        }
    }

    return serialized;
}

auto Contexts::Store(const proto::Context& data, const std::string& alias)
    -> bool
{
    return store_proto(data, data.remotenym(), alias);
}
}  // namespace storage
}  // namespace opentxs
