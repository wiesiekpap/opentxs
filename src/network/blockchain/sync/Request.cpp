// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Request.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"

namespace opentxs::network::blockchain::sync
{
struct RequestImp final : public Base::Imp {
    const Request& parent_;

    auto asRequest() const noexcept -> const Request& final { return parent_; }

    RequestImp(const Request& parent, Request::StateData state) noexcept
        : Imp(Imp::default_version_,
              MessageType::sync_request,
              std::move(state),
              {},
              {})
        , parent_(parent)
    {
    }

private:
    RequestImp() noexcept;
    RequestImp(const RequestImp&) = delete;
    RequestImp(RequestImp&&) = delete;
    auto operator=(const RequestImp&) -> RequestImp& = delete;
    auto operator=(RequestImp&&) -> RequestImp& = delete;
};

Request::Request(StateData in) noexcept
    : Base(std::make_unique<RequestImp>(*this, std::move(in)))
{
}

Request::Request() noexcept
    : Base(std::make_unique<Imp>())
{
}

auto Request::State() const noexcept -> const StateData&
{
    return imp_->state_;
}

Request::~Request() = default;
}  // namespace opentxs::network::blockchain::sync
