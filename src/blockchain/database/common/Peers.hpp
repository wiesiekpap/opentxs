// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstring>
#include <iosfwd>
#include <mutex>
#include <stdexcept>

#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/LMDB.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class Peers
{
public:
    auto Find(
        const Chain chain,
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept -> Address_p;

    auto Import(UnallocatedVector<Address_p> peers) noexcept -> bool;
    auto Insert(Address_p&& address) noexcept -> bool;

    Peers(const api::Session& api, storage::lmdb::LMDB& lmdb) noexcept(false);

private:
    using ChainIndexMap =
        UnallocatedMap<Chain, UnallocatedSet<UnallocatedCString>>;
    using ProtocolIndexMap =
        UnallocatedMap<Protocol, UnallocatedSet<UnallocatedCString>>;
    using ServiceIndexMap =
        UnallocatedMap<Service, UnallocatedSet<UnallocatedCString>>;
    using TypeIndexMap =
        UnallocatedMap<Type, UnallocatedSet<UnallocatedCString>>;
    using ConnectedIndexMap = UnallocatedMap<UnallocatedCString, Time>;

    const api::Session& api_;
    storage::lmdb::LMDB& lmdb_;
    mutable std::mutex lock_;
    ChainIndexMap chains_;
    ProtocolIndexMap protocols_;
    ServiceIndexMap services_;
    TypeIndexMap networks_;
    ConnectedIndexMap connected_;

    auto insert(const Lock& lock, UnallocatedVector<Address_p>&& peers) noexcept
        -> bool;
    auto load_address(const UnallocatedCString& id) const noexcept(false)
        -> Address_p;
    template <typename Index, typename Map>
    auto read_index(
        const ReadView key,
        const ReadView value,
        Map& map) noexcept(false) -> bool
    {
        auto input = std::size_t{};

        if (sizeof(input) != key.size()) {
            throw std::runtime_error("Invalid key");
        }

        std::memcpy(&input, key.data(), key.size());
        map[static_cast<Index>(input)].emplace(value);

        return true;
    }
};
}  // namespace opentxs::blockchain::database::common
