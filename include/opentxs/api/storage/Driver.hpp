// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_STORAGE_DRIVER_HPP
#define OPENTXS_API_STORAGE_DRIVER_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

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
    virtual bool EmptyBucket(const bool bucket) const = 0;

    virtual bool Load(
        const std::string& key,
        const bool checking,
        std::string& value) const = 0;
    virtual bool LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const = 0;

    virtual bool Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const = 0;
    virtual void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const = 0;
    virtual bool Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const = 0;

    virtual bool Migrate(const std::string& key, const Driver& to) const = 0;

    virtual std::string LoadRoot() const = 0;
    virtual bool StoreRoot(const bool commit, const std::string& hash)
        const = 0;

    virtual ~Driver() = default;

    template <class T>
    bool LoadProto(
        const std::string& hash,
        std::shared_ptr<T>& serialized,
        const bool checking = false) const;

    template <class T>
    bool StoreProto(const T& data, std::string& key, std::string& plaintext)
        const;

    template <class T>
    bool StoreProto(const T& data, std::string& key) const;

    template <class T>
    bool StoreProto(const T& data) const;

protected:
    Driver() = default;

private:
    Driver(const Driver&) = delete;
    Driver(Driver&&) = delete;
    Driver& operator=(const Driver&) = delete;
    Driver& operator=(Driver&&) = delete;
};
}  // namespace storage
}  // namespace api
}  // namespace opentxs
#endif
