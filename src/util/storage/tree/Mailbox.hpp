// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <mutex>
#include <string>

#include "Proto.hpp"
#include "internal/util/Editor.hpp"
#include "serialization/protobuf/StorageNymList.pb.h"
#include "util/storage/tree/Node.hpp"

namespace opentxs
{
namespace storage
{
class Driver;
class Nym;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage
{
class Mailbox final : public Node
{
private:
    friend Nym;

    void init(const std::string& hash) final;
    auto save(const std::unique_lock<std::mutex>& lock) const -> bool final;
    auto serialize() const -> proto::StorageNymList;

    Mailbox(const Driver& storage, const std::string& hash);
    Mailbox() = delete;
    Mailbox(const Mailbox&) = delete;
    Mailbox(Mailbox&&) = delete;
    auto operator=(const Mailbox&) -> Mailbox = delete;
    auto operator=(Mailbox&&) -> Mailbox = delete;

public:
    auto Load(
        const std::string& id,
        std::string& output,
        std::string& alias,
        const bool checking) const -> bool;

    auto Delete(const std::string& id) -> bool;
    auto Store(
        const std::string& id,
        const std::string& data,
        const std::string& alias) -> bool;

    ~Mailbox() final = default;
};
}  // namespace opentxs::storage
