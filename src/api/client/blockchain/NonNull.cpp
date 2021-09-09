// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "api/client/blockchain/Blockchain.hpp"  // IWYU pragma: associated

#include "api/client/blockchain/Imp_blockchain.hpp"

namespace opentxs::api::client::implementation
{
Blockchain::Blockchain(
    const api::Core& api,
    const api::client::Activity& activity,
    const api::client::Contacts& contacts,
    const api::Legacy& legacy,
    const std::string& dataFolder,
    const Options& args) noexcept
    : imp_(std::make_unique<BlockchainImp>(
          api,
          activity,
          contacts,
          legacy,
          dataFolder,
          args,
          *this))
{
    // WARNING: do not access api_.Wallet() during construction
}
}  // namespace opentxs::api::client::implementation
