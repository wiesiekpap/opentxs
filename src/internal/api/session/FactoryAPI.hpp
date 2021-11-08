// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/api/FactoryAPI.hpp"

#include "opentxs/api/session/Factory.hpp"

namespace opentxs
{
namespace api
{
namespace crypto
{
class Asymmetric;
class Symmetric;
}  // namespace crypto
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::session::internal
{
class Factory : virtual public api::session::Factory,
                virtual public api::internal::Factory
{
public:
    virtual auto Asymmetric() const -> const api::crypto::Asymmetric& = 0;
    auto InternalSession() const noexcept -> const Factory& final
    {
        return *this;
    }
    virtual auto Symmetric() const -> const api::crypto::Symmetric& = 0;

    auto InternalSession() noexcept -> Factory& final { return *this; }

    ~Factory() override = default;
};
}  // namespace opentxs::api::session::internal
