// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Acknowledgement.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <utility>

#include "network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"

namespace opentxs::network::blockchain::sync
{
struct AckImp final : public Base::Imp {
    const Acknowledgement& parent_;

    auto asAcknowledgement() const noexcept -> const Acknowledgement& final
    {
        return parent_;
    }

    AckImp(
        const Acknowledgement& parent,
        Acknowledgement::StateData state,
        std::string endpoint) noexcept
        : Imp(Imp::default_version_,
              MessageType::sync_ack,
              std::move(state),
              std::move(endpoint),
              {})
        , parent_(parent)
    {
    }

private:
    AckImp() noexcept;
    AckImp(const AckImp&) = delete;
    AckImp(AckImp&&) = delete;
    auto operator=(const AckImp&) -> AckImp& = delete;
    auto operator=(AckImp&&) -> AckImp& = delete;
};

Acknowledgement::Acknowledgement(StateData in, std::string endpoint) noexcept
    : Base(std::make_unique<AckImp>(*this, std::move(in), std::move(endpoint)))
{
}

Acknowledgement::Acknowledgement() noexcept
    : Base(std::make_unique<Imp>())
{
}

auto Acknowledgement::Endpoint() const noexcept -> const std::string&
{
    return imp_->endpoint_;
}

auto Acknowledgement::State() const noexcept -> const StateData&
{
    return imp_->state_;
}

auto Acknowledgement::State(opentxs::blockchain::Type chain) const
    noexcept(false) -> const sync::State&
{
    for (const auto& state : imp_->state_) {
        if (state.Chain() == chain) { return state; }
    }

    throw std::out_of_range{
        "specified chain does not exist in acknowledgement"};
}

Acknowledgement::~Acknowledgement() = default;
}  // namespace opentxs::network::blockchain::sync
