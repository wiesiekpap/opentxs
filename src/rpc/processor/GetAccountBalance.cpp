// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "rpc/RPC.tpp"     // IWYU pragma: associated

#include <string>
#include <vector>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/rpc/AccountType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "rpc/RPC.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::get_account_balance(const request::Base& base) const -> response::Base
{
    const auto& in = base.asGetAccountBalance();
    auto codes = response::Base::Responses{};
    auto balances = response::GetAccountBalance::Data{};
    const auto reply = [&] {
        return response::GetAccountBalance{
            in, std::move(codes), std::move(balances)};
    };

    try {
        const auto& api = session(base);

        for (const auto& id : in.Accounts()) {
            const auto index = codes.size();

            if (id.empty()) {
                codes.emplace_back(index, ResponseCode::invalid);

                continue;
            }

            const auto accountID = api.Factory().Identifier(id);
            const auto account = api.Wallet().Account(accountID);

            if (account) {
                balances.emplace_back(
                    id,
                    account.get().Alias(),
                    account.get().GetInstrumentDefinitionID().str(),
                    api.Storage().AccountOwner(accountID)->str(),
                    api.Storage().AccountIssuer(accountID)->str(),
                    account.get().GetBalance(),
                    account.get().GetBalance(),
                    (account.get().IsIssuer()) ? AccountType::issuer
                                               : AccountType::normal);
                codes.emplace_back(index, ResponseCode::success);
            } else {
                codes.emplace_back(index, ResponseCode::account_not_found);

                continue;
            }
        }
    } catch (...) {
        codes.emplace_back(0, ResponseCode::bad_session);
    }

    return reply();
}
}  // namespace opentxs::rpc::implementation
