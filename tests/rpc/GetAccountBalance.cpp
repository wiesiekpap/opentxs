// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <atomic>
#include <iterator>

#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "ui/Helpers.hpp"

namespace ot = opentxs;
namespace rpc = opentxs::rpc;

namespace ottest
{
constexpr auto issuer_{"otysEksLyHmDZyPDTM4TYvcXEcu18bnZw5V"};
constexpr auto brian_{"ot22MuKFsBJrjjubXRGK83RRruTxtCefLzdD"};
constexpr auto chris_{"ot2C1sgqWjrt6sMJcY2SJLLcB3qMCgJQRmxU"};

TEST_F(RPC_fixture, preconditions)
{
    {
        const auto& session = StartServer(0);
        const auto instance = session.Instance();
        const auto& seeds = seed_map_.at(instance);
        const auto& nyms = local_nym_map_.at(instance);

        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
    }
    {
        const auto& server = ot_.Server(0);
        const auto& session = StartClient(0);
        const auto instance = session.Instance();
        const auto& nyms = local_nym_map_.at(instance);
        const auto& seeds = seed_map_.at(instance);
        const auto seed = ImportBip39(session, GetVectors3().alice_.words_);

        EXPECT_FALSE(seed.empty());
        EXPECT_TRUE(SetIntroductionServer(session, server));

        const auto issuer = CreateNym(session, "issuer", seed, 0);
        const auto brian = CreateNym(session, "brian", seed, 1);
        const auto chris = CreateNym(session, "chris", seed, 2);

        EXPECT_EQ(issuer, issuer_);
        EXPECT_EQ(brian, brian_);
        EXPECT_EQ(chris, chris_);
        EXPECT_TRUE(RegisterNym(session, server, issuer));
        EXPECT_TRUE(RegisterNym(session, server, brian));
        EXPECT_TRUE(RegisterNym(session, server, chris));

        const auto unit = IssueUnit(
            session,
            server,
            issuer,
            "Mt Gox USD",
            "dollars",
            "$",
            "YOLO",
            "USD",
            "cents",
            2,
            ot::contact::ContactItemType::USD);

        EXPECT_FALSE(unit.empty());

        const auto account1 =
            RegisterAccount(session, server, brian, unit, "brian's dollars");
        const auto account2 =
            RegisterAccount(session, server, chris, unit, "chris's dollars1");
        const auto account3 =
            RegisterAccount(session, server, chris, unit, "chris's dollars2");

        EXPECT_FALSE(account1.empty());
        EXPECT_FALSE(account2.empty());
        EXPECT_FALSE(account3.empty());
        EXPECT_TRUE(SendCheque(
            session,
            server,
            issuer,
            registered_accounts_[issuer].front(),
            session.Contacts().ContactID(session.Factory().NymID(brian))->str(),
            "memo1",
            100));
        EXPECT_GT(DepositCheques(session, server, brian), 0);
        EXPECT_TRUE(SendTransfer(
            session, server, brian, account1, account2, "memo2", 42));
        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 3);
        EXPECT_EQ(created_units_.size(), 1);

        RefreshAccount(session, {issuer, brian, chris}, server.ID());
        auto issuerModel = Counter{};
        auto brianModel = Counter{};
        auto chrisModel1 = Counter{};
        auto chrisModel2 = Counter{};
        issuerModel.expected_ += 3;
        brianModel.expected_ += 4;
        chrisModel1.expected_ += 3;
        chrisModel2.expected_ += 1;
        InitAccountActivityCounter(
            session,
            issuer,
            registered_accounts_.at(issuer).front(),
            &issuerModel);
        InitAccountActivityCounter(session, brian, account1, &brianModel);
        InitAccountActivityCounter(session, chris, account2, &chrisModel1);
        InitAccountActivityCounter(session, chris, account3, &chrisModel2);

        EXPECT_TRUE(wait_for_counter(issuerModel));
        EXPECT_TRUE(wait_for_counter(brianModel));
        EXPECT_TRUE(wait_for_counter(chrisModel1));
        EXPECT_TRUE(wait_for_counter(chrisModel2));
    }
}

TEST_F(RPC_fixture, bad_client_session)
{
    constexpr auto index{88};
    const auto accounts = [&] {
        auto out = ot::rpc::request::Base::Identifiers{};
        out.emplace_back(registered_accounts_.at(issuer_).front());

        return out;
    }();
    const auto command = ot::rpc::request::GetAccountBalance{index, accounts};
    const auto& list = command.asGetAccountBalance();
    const auto base = ot_.RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& balances = response.Balances();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::get_account_balance);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::bad_session);
    EXPECT_EQ(balances.size(), 0);
}

TEST_F(RPC_fixture, bad_server_session)
{
    constexpr auto index{99};
    const auto accounts = [&] {
        auto out = ot::rpc::request::Base::Identifiers{};
        out.emplace_back(registered_accounts_.at(issuer_).front());

        return out;
    }();
    const auto command = ot::rpc::request::GetAccountBalance{index, accounts};
    const auto& list = command.asGetAccountBalance();
    const auto base = ot_.RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& balances = response.Balances();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::get_account_balance);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::bad_session);
    EXPECT_EQ(balances.size(), 0);
}

TEST_F(RPC_fixture, all_accounts)
{
    constexpr auto index{0};
    const auto accounts = [&] {
        auto out = ot::rpc::request::Base::Identifiers{};
        const auto& i = registered_accounts_.at(issuer_);
        const auto& b = registered_accounts_.at(brian_);
        const auto& c = registered_accounts_.at(chris_);
        std::copy(i.begin(), i.end(), std::back_inserter(out));
        std::copy(b.begin(), b.end(), std::back_inserter(out));
        std::copy(c.begin(), c.end(), std::back_inserter(out));

        return out;
    }();
    const auto command = ot::rpc::request::GetAccountBalance{index, accounts};
    const auto& list = command.asGetAccountBalance();
    const auto base = ot_.RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& balances = response.Balances();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::get_account_balance);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(base->Cookie(), command.Cookie());
    EXPECT_EQ(base->Session(), command.Session());
    EXPECT_EQ(base->Type(), command.Type());
    EXPECT_EQ(base->Version(), command.Version());
    EXPECT_EQ(response.Cookie(), base->Cookie());
    EXPECT_EQ(response.ResponseCodes(), base->ResponseCodes());
    EXPECT_EQ(response.Session(), base->Session());
    EXPECT_EQ(response.Type(), base->Type());
    EXPECT_EQ(response.Version(), base->Version());
    EXPECT_EQ(response.Version(), command.Version());
    EXPECT_EQ(response.Cookie(), command.Cookie());
    EXPECT_EQ(response.Session(), command.Session());
    EXPECT_EQ(response.Type(), command.Type());
    ASSERT_EQ(codes.size(), 4);

    for (auto i{0u}; i < codes.size(); ++i) {
        const auto& [index, code] = codes.at(i);
        const auto expected = rpc::ResponseCode::success;

        EXPECT_EQ(index, i);
        EXPECT_EQ(code, expected);
    }

    EXPECT_EQ(balances.size(), 4);

    // TODO verify each item in balances
}

// TODO test other combinations of accounts
// TODO track down mystery
// "opentxs::ui::implementation::TransferBalanceItem::startup: Invalid event
// state (4)" log message

TEST_F(RPC_fixture, cleanup) { Cleanup(); }
}  // namespace ottest
