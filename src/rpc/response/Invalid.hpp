// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/rpc/response/Base.hpp"

namespace opentxs
{
namespace rpc
{
namespace request
{
class Base;
}  // namespace request
}  // namespace rpc
}  // namespace opentxs

namespace opentxs::rpc::response
{
struct Invalid final : Base {
    Invalid(const request::Base& request) noexcept;

    ~Invalid() final;

private:
    Invalid() = delete;
    Invalid(const Invalid&) = delete;
    Invalid(Invalid&&) = delete;
    auto operator=(const Invalid&) -> Invalid& = delete;
    auto operator=(Invalid&&) -> Invalid& = delete;
};
}  // namespace opentxs::rpc::response
