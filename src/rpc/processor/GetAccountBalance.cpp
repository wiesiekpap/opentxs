// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "rpc/RPC.tpp"     // IWYU pragma: associated

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include "display/Definition.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/core/Core.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/contract/UnitDefinition.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/rpc/AccountData.hpp"
#include "opentxs/rpc/AccountType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "rpc/RPC.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::get_account_balance(const request::Base& base) const noexcept
    -> std::unique_ptr<response::Base>
{
    const auto& in = base.asGetAccountBalance();
    auto codes = response::Base::Responses{};
    auto balances = response::GetAccountBalance::Data{};
    const auto reply = [&] {
        return std::make_unique<response::GetAccountBalance>(
            in, std::move(codes), std::move(balances));
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

            if (is_blockchain_account(base, accountID)) {
                get_account_balance_blockchain(
                    base, index, accountID, balances, codes);
            } else {
                get_account_balance_custodial(
                    api, index, accountID, balances, codes);
            }
        }
    } catch (...) {
        codes.emplace_back(0, ResponseCode::bad_session);
    }

    return reply();
}

auto RPC::get_account_balance_blockchain(
    const request::Base& base,
    const std::size_t index,
    const Identifier& accountID,
    std::vector<AccountData>& balances,
    response::Base::Responses& codes) const noexcept -> void
{
    try {
        const auto& api = client_session(base);
        const auto& blockchain = api.Blockchain();
        const auto [chain, owner] = blockchain.LookupAccount(accountID);
        api.Network().Blockchain().Start(chain);
        const auto& client = api.Network().Blockchain().GetChain(chain);
        const auto [confirmed, unconfirmed] = client.GetBalance(owner);
        const auto& display =
            blockchain::params::Data::Chains().at(chain).scales_;
        balances.emplace_back(
            accountID.str(),
            blockchain::AccountName(chain),
            blockchain::UnitID(api, chain).str(),
            owner->str(),
            blockchain::IssuerID(api, chain).str(),
            display.Format(confirmed),
            display.Format(unconfirmed),
            confirmed,
            unconfirmed,
            AccountType::blockchain);
        codes.emplace_back(index, ResponseCode::success);
    } catch (...) {
        codes.emplace_back(index, ResponseCode::account_not_found);
    }
}

auto RPC::get_account_balance_custodial(
    const api::Core& api,
    const std::size_t index,
    const Identifier& accountID,
    std::vector<AccountData>& balances,
    response::Base::Responses& codes) const noexcept -> void
{
    const auto account = api.Wallet().Account(accountID);

    if (account) {
        const auto& unit = account.get().GetInstrumentDefinitionID();
        const auto balance = account.get().GetBalance();
        const auto formatted = [&] {
            auto out = std::string{};
            const auto contract = api.Wallet().UnitDefinition(unit);
            contract->FormatAmountLocale(balance, out);

            return out;
        }();
        balances.emplace_back(
            accountID.str(),
            account.get().Alias(),
            unit.str(),
            api.Storage().AccountOwner(accountID)->str(),
            api.Storage().AccountIssuer(accountID)->str(),
            formatted,
            formatted,
            balance,
            balance,
            (account.get().IsIssuer()) ? AccountType::issuer
                                       : AccountType::normal);
        codes.emplace_back(index, ResponseCode::success);
    } else {
        codes.emplace_back(index, ResponseCode::account_not_found);
    }
}
}  // namespace opentxs::rpc::implementation
