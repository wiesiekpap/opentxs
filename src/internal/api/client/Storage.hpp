// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/api/session/Storage.hpp"

namespace opentxs
{
namespace crypto
{
namespace key
{
class Symmetric;
}  // namespace key
}  // namespace crypto
}  // namespace opentxs

namespace opentxs::api::storage::internal
{
class Storage : virtual public storage::Storage
{
public:
    virtual void InitBackup() = 0;
    virtual void InitEncryptedBackup(opentxs::crypto::key::Symmetric& key) = 0;
    virtual void start() = 0;

    virtual ~Storage() override = default;

protected:
    Storage() = default;

private:
    Storage(const Storage&) = delete;
    Storage(Storage&&) = delete;
    auto operator=(const Storage&) -> Storage& = delete;
    auto operator=(Storage&&) -> Storage& = delete;
};
}  // namespace opentxs::api::storage::internal
