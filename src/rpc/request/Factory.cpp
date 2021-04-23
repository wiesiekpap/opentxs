// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "1_Internal.hpp"                // IWYU pragma: associated
#include "opentxs/rpc/request/Base.hpp"  // IWYU pragma: associated

#include <stdexcept>

#include "internal/rpc/RPC.hpp"
#include "opentxs/Proto.tpp"
#include "opentxs/protobuf/Check.hpp"
#include "opentxs/protobuf/RPCCommand.pb.h"
#include "opentxs/protobuf/verify/RPCCommand.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/request/ListNyms.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"

namespace opentxs::rpc::request
{
auto Factory(const proto::RPCCommand& proto) noexcept -> Base
{
    try {
        if (false == proto::Validate(proto, VERBOSE)) {
            throw std::runtime_error{"invalid serialized rpc request"};
        }

        switch (translate(proto.type())) {
            case CommandType::get_account_activity: {

                return GetAccountActivity{proto};
            }
            case CommandType::get_account_balance: {

                return GetAccountBalance{proto};
            }
            case CommandType::list_accounts: {

                return ListAccounts{proto};
            }
            case CommandType::list_nyms: {

                return ListNyms{proto};
            }
            case CommandType::send_payment: {

                return SendPayment{proto};
            }
            default: {

                return {};
            }
        }
    } catch (...) {

        return {};
    }
}

auto Factory(ReadView serialized) noexcept -> Base
{
    return Factory(proto::Factory<proto::RPCCommand>(serialized));
}
}  // namespace opentxs::rpc::request
