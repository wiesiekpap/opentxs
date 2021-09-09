// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "storage/tree/Servers.hpp"  // IWYU pragma: associated

#include <cstdlib>
#include <iostream>
#include <map>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/ServerContract.pb.h"
#include "opentxs/protobuf/StorageItemHash.pb.h"
#include "opentxs/protobuf/StorageServers.pb.h"
#include "opentxs/protobuf/verify/ServerContract.hpp"
#include "opentxs/protobuf/verify/StorageServers.hpp"
#include "storage/Plugin.hpp"
#include "storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{
Servers::Servers(
    const opentxs::api::storage::Driver& storage,
    const std::string& hash)
    : Node(storage, hash)
{
    if (check_hash(hash)) {
        init(hash);
    } else {
        blank(2);
    }
}

auto Servers::Alias(const std::string& id) const -> std::string
{
    return get_alias(id);
}

auto Servers::Delete(const std::string& id) -> bool { return delete_item(id); }

void Servers::init(const std::string& hash)
{
    std::shared_ptr<proto::StorageServers> serialized;
    driver_.LoadProto(hash, serialized);

    if (!serialized) {
        std::cerr << __func__ << ": Failed to load servers index file."
                  << std::endl;
        abort();
    }

    init_version(2, *serialized);

    for (const auto& it : serialized->server()) {
        item_map_.emplace(
            it.itemid(), Metadata{it.hash(), it.alias(), 0, false});
    }
}

auto Servers::Load(
    const std::string& id,
    std::shared_ptr<proto::ServerContract>& output,
    std::string& alias,
    const bool checking) const -> bool
{
    return load_proto<proto::ServerContract>(id, output, alias, checking);
}

void Servers::Map(ServerLambda lambda) const
{
    map<proto::ServerContract>(lambda);
}

auto Servers::save(const std::unique_lock<std::mutex>& lock) const -> bool
{
    if (!verify_write_lock(lock)) {
        std::cerr << __func__ << ": Lock failure." << std::endl;
        abort();
    }

    auto serialized = serialize();

    if (!proto::Validate(serialized, VERBOSE)) { return false; }

    return driver_.StoreProto(serialized, root_);
}

auto Servers::serialize() const -> proto::StorageServers
{
    proto::StorageServers serialized;
    serialized.set_version(version_);

    for (const auto& item : item_map_) {
        const bool goodID = !item.first.empty();
        const bool goodHash = check_hash(std::get<0>(item.second));
        const bool good = goodID && goodHash;

        if (good) {
            serialize_index(
                version_, item.first, item.second, *serialized.add_server());
        }
    }

    return serialized;
}

auto Servers::SetAlias(const std::string& id, const std::string& alias) -> bool
{
    return set_alias(id, alias);
}

auto Servers::Store(
    const proto::ServerContract& data,
    const std::string& alias,
    std::string& plaintext) -> bool
{
    return store_proto(data, data.id(), alias, plaintext);
}
}  // namespace storage
}  // namespace opentxs
