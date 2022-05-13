// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/database/Header.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/database/Sync.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/Wallet.hpp"

namespace opentxs::blockchain::database
{
class Database : virtual public database::Block,
                 virtual public database::Cfilter,
                 virtual public database::Header,
                 virtual public database::Peer,
                 virtual public database::Sync,
                 virtual public database::Wallet
{
public:
    ~Database() override = default;
};
}  // namespace opentxs::blockchain::database
