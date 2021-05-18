// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <future>
#include <memory>
#include <string>

#include "Proto.hpp"
#include "Proto.tpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/storage/Driver.hpp"
#include "opentxs/api/storage/Plugin.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/LogSource.hpp"
#include "opentxs/protobuf/Check.hpp"

namespace opentxs
{
namespace api
{
namespace storage
{
class Storage;
}  // namespace storage
}  // namespace api

class Flag;
class StorageConfig;

class Plugin : virtual public opentxs::api::storage::Plugin
{
public:
    auto EmptyBucket(const bool bucket) const -> bool override = 0;

    auto Load(const std::string& key, const bool checking, std::string& value)
        const -> bool override;
    auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool override = 0;
    auto Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const -> bool override;
    void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const override;
    auto Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const -> bool override;

    auto Migrate(
        const std::string& key,
        const opentxs::api::storage::Driver& to) const -> bool override;

    auto LoadRoot() const -> std::string override = 0;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool override = 0;

    virtual void Cleanup() = 0;

    ~Plugin() override = default;

protected:
    const StorageConfig& config_;
    const Random& random_;

    Plugin(
        const api::storage::Storage& storage,
        const StorageConfig& config,
        const Digest& hash,
        const Random& random,
        const Flag& bucket);
    Plugin() = delete;

    virtual void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const = 0;

private:
    const api::storage::Storage& storage_;
    const Digest& digest_;
    const Flag& current_bucket_;

    Plugin(const Plugin&) = delete;
    Plugin(Plugin&&) = delete;
    auto operator=(const Plugin&) -> Plugin& = delete;
    auto operator=(Plugin&&) -> Plugin& = delete;
};

template <class T>
auto opentxs::api::storage::Driver::LoadProto(
    const std::string& hash,
    std::shared_ptr<T>& serialized,
    const bool checking) const -> bool
{
    auto raw = std::string{};
    const auto loaded = Load(hash, checking, raw);
    auto valid{false};

    if (loaded) {
        serialized = proto::DynamicFactory<T>(raw.data(), raw.size());

        OT_ASSERT(serialized);

        valid = proto::Validate<T>(*serialized, VERBOSE);
    } else {

        return false;
    }

    if (!valid) {
        if (loaded) {
            LogOutput(": Specified object was located but could not be "
                      "validated.")
                .Flush();
            LogOutput(": Hash: ")(hash).Flush();
            LogOutput(": Size: ")(raw.size()).Flush();
        } else {

            LogDetail("Specified object is missing.").Flush();
            LogDetail("Hash: ")(hash).Flush();
            LogDetail("Size: ")(raw.size()).Flush();
        }
    }

    OT_ASSERT(valid);

    return valid;
}

template <class T>
auto opentxs::api::storage::Driver::StoreProto(
    const T& data,
    std::string& key,
    std::string& plaintext) const -> bool
{
    if (!proto::Validate<T>(data, VERBOSE)) { return false; }

    plaintext = proto::ToString(data);

    return Store(true, plaintext, key);
}

template <class T>
auto opentxs::api::storage::Driver::StoreProto(const T& data, std::string& key)
    const -> bool
{
    std::string notUsed;

    return StoreProto<T>(data, key, notUsed);
}

template <class T>
auto opentxs::api::storage::Driver::StoreProto(const T& data) const -> bool
{
    std::string notUsed;

    return StoreProto<T>(data, notUsed);
}
}  // namespace opentxs
