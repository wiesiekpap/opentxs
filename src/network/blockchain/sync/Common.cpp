// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Types.hpp"  // IWYU pragma: associated

#include <boost/container/flat_map.hpp>
#include <boost/intrusive/detail/iterator.hpp>
#include <boost/move/algo/detail/set_difference.hpp>
#include <boost/move/algo/move.hpp>
#include <functional>
#include <memory>

#include "opentxs/network/blockchain/sync/MessageType.hpp"

namespace opentxs
{
using Type = network::blockchain::sync::MessageType;

auto print(Type value) noexcept -> std::string
{
    static const auto map = boost::container::flat_map<Type, std::string>{
        {Type::sync_request, "sync request"},
        {Type::sync_ack, "sync acknowledgment"},
        {Type::sync_reply, "sync reply"},
        {Type::new_block_header, "sync push"},
        {Type::query, "sync query"},
    };

    try {

        return map.at(value);
    } catch (...) {

        return "error";
    }
}
}  // namespace opentxs
