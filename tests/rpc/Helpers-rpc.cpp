// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <future>
#include <iterator>
#include <mutex>
#include <utility>

#include "integration/Helpers.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/Unit.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/interface/rpc/AccountData.hpp"
#include "opentxs/interface/rpc/AccountEvent.hpp"
#include "opentxs/interface/rpc/ResponseCode.hpp"
#include "opentxs/interface/rpc/request/GetAccountActivity.hpp"
#include "opentxs/interface/rpc/request/GetAccountBalance.hpp"
#include "opentxs/interface/rpc/request/ListAccounts.hpp"
#include "opentxs/interface/rpc/response/Base.hpp"
#include "opentxs/interface/rpc/response/GetAccountActivity.hpp"
#include "opentxs/interface/rpc/response/GetAccountBalance.hpp"
#include "opentxs/interface/rpc/response/ListAccounts.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
auto verify_account_balance(
    const int index,
    const ot::UnallocatedCString& account,
    const ot::Amount required) noexcept -> bool;
auto verify_response_codes(
    const ot::rpc::response::Base::Responses& codes,
    const std::size_t count,
    const ot::rpc::ResponseCode required =
        ot::rpc::ResponseCode::success) noexcept -> bool;

auto check_account_activity_rpc(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool
{
    auto output{true};
    const auto& api = *user.api_;
    const auto index{api.Instance()};
    const auto command =
        ot::rpc::request::GetAccountActivity{index, {account.str()}};
    const auto base = ot::Context().RPC(command);
    const auto& response = base->asGetAccountActivity();
    const auto& codes = response.ResponseCodes();
    const auto& events = response.Activity();
    const auto vCount = expected.rows_.size();
    const auto goodCodes = verify_response_codes(
        codes,
        1,
        (0 == vCount) ? ot::rpc::ResponseCode::none
                      : ot::rpc::ResponseCode::success);
    output &= verify_account_balance(index, account.str(), expected.balance_);
    output &= goodCodes;
    output &= (events.size() == vCount);

    EXPECT_EQ(events.size(), vCount);

    auto v{expected.rows_.begin()};
    auto t{events.begin()};

    for (; (v != expected.rows_.end()) && (t != events.end()); ++v, ++t) {
        const auto& event = *t;
        const auto& required = *v;

        // TODO event.ContactID()
        // TODO event.State()
        // TODO event.Type()
        output &= (event.AccountID() == expected.id_);
        output &= (event.ConfirmedAmount() == required.amount_);
        output &= (event.Memo() == required.memo_);
        output &= (event.PendingAmount() == required.amount_);
        output &= (event.UUID() == required.uuid_);
        output &= (event.WorkflowID() == required.workflow_);

        EXPECT_EQ(event.AccountID(), expected.id_);
        EXPECT_EQ(event.ConfirmedAmount(), required.amount_);
        EXPECT_EQ(event.Memo(), required.memo_);
        EXPECT_EQ(event.PendingAmount(), required.amount_);
        EXPECT_EQ(event.UUID(), required.uuid_);
        EXPECT_EQ(event.WorkflowID(), required.workflow_);
    }

    return output;
}

auto check_account_list_rpc(
    const User& user,
    const AccountListData& expected) noexcept -> bool
{
    auto output{true};
    const auto& api = *user.api_;
    const auto index{api.Instance()};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto base = ot::Context().RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();
    const auto goodCodes = verify_response_codes(codes, 1);
    const auto required = [&] {
        auto out = ot::UnallocatedVector<ot::UnallocatedCString>{};
        out.reserve(expected.rows_.size());

        for (const auto& row : expected.rows_) {
            out.emplace_back(row.account_id_);
        }

        std::sort(out.begin(), out.end());

        return out;
    }();
    const auto got = [&] {
        auto out{ids};
        std::sort(out.begin(), out.end());

        return out;
    }();
    output &= goodCodes;
    output &= (got == required);

    EXPECT_TRUE(goodCodes);
    EXPECT_EQ(got, required);

    return output;
}

auto verify_account_balance(
    const int index,
    const ot::UnallocatedCString& account,
    const ot::Amount required) noexcept -> bool
{
    auto output{true};
    const auto command = ot::rpc::request::GetAccountBalance{index, {account}};
    const auto base = ot::Context().RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& data = response.Balances();
    const auto goodCodes = verify_response_codes(codes, 1);
    output &= goodCodes;
    output &= (data.size() == 1);

    EXPECT_TRUE(goodCodes);
    EXPECT_EQ(data.size(), 1);

    if (0 < data.size()) {
        const auto& item = data.front();

        // TODO item.ConfirmedBalance()
        // TODO item.ID()
        // TODO item.Issuer()
        // TODO item.Name()
        // TODO item.Owner()
        // TODO item.Type()
        // TODO item.Unit()

        output &= (item.PendingBalance() == required);

        EXPECT_EQ(item.PendingBalance(), required);
    }

    return output;
}

auto verify_response_codes(
    const ot::rpc::response::Base::Responses& codes,
    const std::size_t count,
    const ot::rpc::ResponseCode required) noexcept -> bool
{
    auto output{true};
    output &= (codes.size() == count);

    EXPECT_EQ(codes.size(), count);

    for (auto i = std::size_t{}; i < codes.size(); ++i) {
        const auto& code = codes.at(i);
        output &= (code.first == i);
        output &= (code.second == required);

        EXPECT_EQ(code.first, i);
        EXPECT_EQ(code.second, required);
    }

    return output;
}
}  // namespace ottest
