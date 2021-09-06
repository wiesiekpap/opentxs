// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "blockchain/p2p/peer/Peer.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::p2p::implementation
{
Peer::Address::Address(std::unique_ptr<internal::Address> address) noexcept
    : lock_()
    , address_(std::move(address))
{
    OT_ASSERT(address_);
}

auto Peer::Address::Bytes() const noexcept -> OTData
{
    Lock lock(lock_);

    return address_->Bytes();
}

auto Peer::Address::Chain() const noexcept -> blockchain::Type
{
    Lock lock(lock_);

    return address_->Chain();
}

auto Peer::Address::Display() const noexcept -> std::string
{
    Lock lock(lock_);

    return address_->Display();
}

auto Peer::Address::ID() const noexcept -> OTIdentifier
{
    Lock lock(lock_);

    return address_->ID();
}

auto Peer::Address::Incoming() const noexcept -> bool
{
    Lock lock(lock_);

    return address_->Incoming();
}

auto Peer::Address::Port() const noexcept -> std::uint16_t
{
    Lock lock(lock_);

    return address_->Port();
}

auto Peer::Address::Services() const noexcept -> std::set<Service>
{
    Lock lock(lock_);

    return address_->Services();
}

auto Peer::Address::Type() const noexcept -> Network
{
    Lock lock(lock_);

    return address_->Type();
}

auto Peer::Address::UpdateServices(
    const std::set<p2p::Service>& services) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetServices(services);

    return address_->clone_internal();
}

auto Peer::Address::UpdateTime(const Time& time) noexcept -> pointer
{
    Lock lock(lock_);
    address_->SetLastConnected(time);

    return address_->clone_internal();
}
}  // namespace opentxs::blockchain::p2p::implementation
