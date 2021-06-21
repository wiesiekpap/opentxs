// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "util/LMDB.hpp"

namespace opentxs
{
namespace storage
{
namespace lmdb
{
class LMDB;
}  // namespace lmdb
}  // namespace storage

namespace util
{
struct IndexData;
}  // namespace util
}  // namespace opentxs

namespace opentxs::blockchain::database::common
{
class Bulk
{
public:
    using UpdateCallback =
        std::function<bool(storage::lmdb::LMDB::Transaction&)>;

    auto Mutex() const noexcept -> std::mutex&;
    auto ReadView(const util::IndexData& index) const noexcept
        -> opentxs::ReadView;
    auto ReadView(const Lock& lock, const util::IndexData& index) const noexcept
        -> opentxs::ReadView;
    auto WriteView(
        storage::lmdb::LMDB::Transaction& tx,
        util::IndexData& index,
        UpdateCallback&& cb,
        std::size_t size) const noexcept -> WritableView;
    auto WriteView(
        const Lock& lock,
        storage::lmdb::LMDB::Transaction& tx,
        util::IndexData& index,
        UpdateCallback&& cb,
        std::size_t size) const noexcept -> WritableView;

    Bulk(storage::lmdb::LMDB& lmdb, const std::string& path) noexcept(false);

    ~Bulk();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Bulk() = delete;
    Bulk(const Bulk&) = delete;
    Bulk(Bulk&&) = delete;
    Bulk& operator=(const Bulk&) = delete;
    Bulk& operator=(Bulk&&) = delete;
};
}  // namespace opentxs::blockchain::database::common
