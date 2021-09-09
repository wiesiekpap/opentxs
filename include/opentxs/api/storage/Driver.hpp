// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_STORAGE_DRIVER_HPP
#define OPENTXS_API_STORAGE_DRIVER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <future>
#include <memory>
#include <string>

namespace opentxs
{
namespace api
{
namespace storage
{
class Driver
{
public:
    virtual auto EmptyBucket(const bool bucket) const -> bool = 0;

    virtual auto Load(
        const std::string& key,
        const bool checking,
        std::string& value) const -> bool = 0;
    virtual auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool = 0;

    virtual auto Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const -> bool = 0;
    virtual void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const = 0;
    virtual auto Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const -> bool = 0;

    virtual auto Migrate(const std::string& key, const Driver& to) const
        -> bool = 0;

    virtual auto LoadRoot() const -> std::string = 0;
    virtual auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool = 0;

    virtual ~Driver() = default;

    template <class T>
    auto LoadProto(
        const std::string& hash,
        std::shared_ptr<T>& serialized,
        const bool checking = false) const -> bool;

    template <class T>
    auto StoreProto(const T& data, std::string& key, std::string& plaintext)
        const -> bool;

    template <class T>
    auto StoreProto(const T& data, std::string& key) const -> bool;

    template <class T>
    auto StoreProto(const T& data) const -> bool;

protected:
    Driver() = default;

private:
    Driver(const Driver&) = delete;
    Driver(Driver&&) = delete;
    auto operator=(const Driver&) -> Driver& = delete;
    auto operator=(Driver&&) -> Driver& = delete;
};
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif
