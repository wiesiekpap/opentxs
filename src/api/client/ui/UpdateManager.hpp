// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "opentxs/Types.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

class Identifier;
}  // namespace opentxs

namespace opentxs::api::client::ui
{
struct UpdateManager {
    auto ActivateUICallback(const Identifier& widget) const noexcept -> void;
    auto ClearUICallbacks(const Identifier& widget) const noexcept -> void;
    auto RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
        const noexcept -> void;

    UpdateManager(const api::client::Manager& api) noexcept;

    ~UpdateManager();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::api::client::ui
