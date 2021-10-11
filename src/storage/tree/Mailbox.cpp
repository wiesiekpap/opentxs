// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "storage/tree/Mailbox.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/StorageEnums.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"
#include "opentxs/protobuf/StorageNymList.pb.h"
#include "opentxs/protobuf/verify/StorageNymList.hpp"
#include "opentxs/storage/Driver.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{
Mailbox::Mailbox(const Driver& storage, const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(2);
    }
}

auto Mailbox::Delete(const std::string& id) -> bool { return delete_item(id); }

void Mailbox::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageNymList> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load mailbox index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->nym()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Mailbox::Load(
    const std::string& id,
    std::string& output,
    std::string& alias,
    const bool checking) const -> bool
{
    return load_raw(id, output, alias, checking);
}

auto Mailbox::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Mailbox::serialize() const -> proto::StorageNymList
{
    proto::StorageNymList serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_,
                item.first,
                item.second,
                *serialized.add_nym(),
                proto::STORAGEHASH_RAW);
        }
    }

    return serialized;
}

auto Mailbox::Store(
    const std::string& id,
    const std::string& data,
    const std::string& alias) -> bool
{
    return store_raw(data, id, alias);
}
}  // namespace storage
}  // namespace opentxs
