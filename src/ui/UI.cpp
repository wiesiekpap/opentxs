// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "internal/ui/UI.hpp"  // IWYU pragma: associated

#include <atomic>

#include "opentxs/Pimpl.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/core/identifier/Nym.hpp"

namespace opentxs::ui::internal
{
auto Row::next_index() noexcept -> std::ptrdiff_t
{
    static auto counter = std::atomic<std::ptrdiff_t>{-1};

    return ++counter;
}
}  // namespace opentxs::ui::internal

namespace opentxs::ui::internal::blank
{
auto blank_nym() noexcept -> const identifier::Nym&;

auto BlockchainSubaccountSource::NymID() const noexcept
    -> const identifier::Nym&
{
    return blank_nym();
}

auto BlockchainSubaccountSource::SourceID() const noexcept -> const Identifier&
{
    return blank_nym();
}

auto BlockchainSubaccountSource::Type() const noexcept
    -> blockchain::crypto::SubaccountType
{
    return blockchain::crypto::SubaccountType::Error;
}

auto BlockchainSubaccount::NymID() const noexcept -> const identifier::Nym&
{
    return blank_nym();
}

auto BlockchainSubaccount::SubaccountID() const noexcept -> const Identifier&
{
    return blank_nym();
}

auto BlockchainSubchain::Type() const noexcept -> blockchain::crypto::Subchain
{
    return blockchain::crypto::Subchain::Error;
}

auto blank_nym() noexcept -> const identifier::Nym&
{
    static const auto data = identifier::Nym::Factory("");

    return data;
}
}  // namespace opentxs::ui::internal::blank
