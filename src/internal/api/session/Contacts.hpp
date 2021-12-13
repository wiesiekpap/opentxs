// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/api/session/Contacts.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::session::internal
{
class Contacts : virtual public api::session::Contacts
{
public:
    auto Internal() const noexcept -> const internal::Contacts& final
    {
        return *this;
    }

    auto Internal() noexcept -> internal::Contacts& final { return *this; }
    virtual auto init(
        const std::shared_ptr<const crypto::Blockchain>& blockchain)
        -> void = 0;
    virtual auto prepare_shutdown() -> void = 0;
    virtual auto start() -> void = 0;

    ~Contacts() override = default;
};
}  // namespace opentxs::api::session::internal
