// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/api/server/Factory.cpp"

#pragma once

#include <memory>

#include "api/Factory.hpp"

namespace opentxs
{
namespace api
{
namespace server
{
namespace internal
{
struct Manager;
}  // namespace internal
}  // namespace server

namespace internal
{
struct Factory;
}  // namespace internal
}  // namespace api

class Factory;
class OTCron;
}  // namespace opentxs

namespace opentxs::api::server::implementation
{
class Factory final : public opentxs::api::implementation::Factory
{
public:
    auto Cron() const -> std::unique_ptr<OTCron> final;

    ~Factory() final = default;

private:
    friend opentxs::Factory;

    const api::server::internal::Manager& server_;

    Factory(const api::server::internal::Manager& server);
    Factory() = delete;
    Factory(const Factory&) = delete;
    Factory(Factory&&) = delete;
    auto operator=(const Factory&) -> Factory& = delete;
    auto operator=(Factory&&) -> Factory& = delete;
};
}  // namespace opentxs::api::server::implementation
