// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <iterator>
#include <list>

#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "paymentcode/VectorsV3.hpp"

namespace ot = opentxs;
namespace rpc = opentxs::rpc;

namespace ottest
{
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
        const auto& session = StartServer(1);
        const auto instance = session.Instance();
        const auto& seeds = seed_map_.at(instance);
        const auto& nyms = local_nym_map_.at(instance);

        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
    }
    {
        const auto& session = StartServer(2);
        const auto instance = session.Instance();
        const auto& seeds = seed_map_.at(instance);
        const auto& nyms = local_nym_map_.at(instance);

        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
    }
    {
        const auto& server1 = ot_.Server(0);
        const auto& server2 = ot_.Server(1);
        const auto& session = StartClient(0);
        const auto instance = session.Instance();
        const auto& nyms = local_nym_map_.at(instance);
        const auto& seeds = seed_map_.at(instance);
        const auto seed = ImportBip39(session, GetVectors3().bob_.words_);

        EXPECT_FALSE(seed.empty());
        EXPECT_TRUE(SetIntroductionServer(session, server1));
        EXPECT_TRUE(ImportServerContract(server2, session));

        const auto issuer = CreateNym(session, "issuer", seed, 0);
        const auto unit1 = IssueUnit(
            session,
            server1,
            issuer,
            "Mt Gox USD",
            "dollars",
            "$",
            "YOLO",
            "USD",
            "cents",
            2,
            ot::contact::ContactItemType::USD);
        const auto unit2 = IssueUnit(
            session,
            server2,
            issuer,
            "Mt Gox BTC",
            "bitcoins",
            "B",
            "YOLO",
            "BTC",
            "satoshis",
            8,
            ot::contact::ContactItemType::BTC);

        EXPECT_FALSE(unit1.empty());
        EXPECT_FALSE(unit2.empty());
        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
        ASSERT_EQ(created_units_.size(), 2);
    }
    {
        const auto& server1 = ot_.Server(0);
        const auto& server2 = ot_.Server(1);
        const auto& session = StartClient(1);
        const auto instance = session.Instance();
        const auto& nyms = local_nym_map_.at(instance);
        const auto& seeds = seed_map_.at(instance);
        const auto seed = ImportBip39(session, GetVectors3().alice_.words_);
        const auto& unit1 = created_units_.at(0);
        const auto& unit2 = created_units_.at(1);

        EXPECT_FALSE(seed.empty());
        EXPECT_TRUE(SetIntroductionServer(session, server1));
        EXPECT_TRUE(ImportServerContract(server2, session));

        const auto brian = CreateNym(session, "brian", seed, 0);
        const auto chris = CreateNym(session, "chris", seed, 1);
        const auto account1 =
            RegisterAccount(session, server1, brian, unit1, "brian's dollars");
        const auto account2 =
            RegisterAccount(session, server2, brian, unit2, "brian's bitcoins");
        const auto account3 =
            RegisterAccount(session, server1, chris, unit1, "chris's dollars");

        EXPECT_FALSE(account1.empty());
        EXPECT_FALSE(account2.empty());
        EXPECT_FALSE(account3.empty());
        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 2);
    }
    {
        const auto& session = StartClient(2);
        const auto instance = session.Instance();
        const auto& nyms = local_nym_map_.at(instance);
        const auto& seeds = seed_map_.at(instance);

        EXPECT_EQ(seeds.size(), 0);
        EXPECT_EQ(nyms.size(), 0);
    }
}

TEST_F(RPC_fixture, bad_client_session)
{
    constexpr auto index{88};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(ids.size(), 0);
}

TEST_F(RPC_fixture, bad_server_session)
{
    constexpr auto index{99};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(ids.size(), 0);
}

TEST_F(RPC_fixture, issuer_accounts)
{
    constexpr auto index{0};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto expected = [&] {
        auto out = std::set<std::string>{};
        const auto& api = ot_.Client(0);
        const auto& issuer = *local_nym_map_.at(0).begin();
        const auto nym = api.Factory().NymID(issuer);
        const auto ids = api.Storage().AccountsByOwner(nym);
        std::transform(
            ids.begin(),
            ids.end(),
            std::inserter(out, out.end()),
            [](const auto& id) { return id->str(); });

        return out;
    }();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::success);
    EXPECT_EQ(ids.size(), 2);

    for (const auto& id : ids) { EXPECT_EQ(expected.count(id), 1); }
}

TEST_F(RPC_fixture, user_accounts)
{
    constexpr auto index{2};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto expected = [&] {
        auto out = std::set<std::string>{};
        const auto& api = ot_.Client(1);
        const auto ids = api.Storage().AccountList();
        std::transform(
            ids.begin(),
            ids.end(),
            std::inserter(out, out.end()),
            [](const auto& id) { return id.first; });

        return out;
    }();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::success);
    EXPECT_EQ(ids.size(), 3);

    for (const auto& id : ids) { EXPECT_EQ(expected.count(id), 1); }
}

TEST_F(RPC_fixture, zero_accounts)
{
    constexpr auto index{4};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::none);
    EXPECT_EQ(ids.size(), 0);
}

TEST_F(RPC_fixture, filter_nym_match)
{
    constexpr auto index{2};
    const auto filterNym = *local_nym_map_.at(index).begin();
    const auto command = ot::rpc::request::ListAccounts{index, filterNym};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto expected = [&] {
        auto out = std::set<std::string>{};
        const auto& api = ot_.Client(1);
        const auto nym = api.Factory().NymID(filterNym);
        const auto ids = api.Storage().AccountsByOwner(nym);
        std::transform(
            ids.begin(),
            ids.end(),
            std::inserter(out, out.end()),
            [](const auto& id) { return id->str(); });

        return out;
    }();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(command.FilterNym(), filterNym);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::success);
    EXPECT_EQ(ids.size(), 1);

    for (const auto& id : ids) { EXPECT_EQ(expected.count(id), 1); }
}

TEST_F(RPC_fixture, filter_nym_no_match)
{
    constexpr auto index{0};
    const auto filterNym = *local_nym_map_.at(2).begin();
    const auto command = ot::rpc::request::ListAccounts{index, filterNym};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(command.FilterNym(), filterNym);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::none);
    EXPECT_EQ(ids.size(), 0);
}

TEST_F(RPC_fixture, filter_server_match)
{
    constexpr auto index{2};
    const auto filterServer = ot_.Server(1).ID().str();
    const auto command =
        ot::rpc::request::ListAccounts{index, "", filterServer};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto owner = *local_nym_map_.at(2).rbegin();
    const auto expected = [&]() -> std::set<std::string> {
        return {registered_accounts_.at(owner).at(1)};
    }();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(command.FilterNotary(), filterServer);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::success);
    EXPECT_EQ(ids.size(), 1);

    for (const auto& id : ids) { EXPECT_EQ(expected.count(id), 1); }
}

TEST_F(RPC_fixture, filter_server_no_match)
{
    constexpr auto index{2};
    const auto filterServer = ot_.Server(2).ID().str();
    const auto command =
        ot::rpc::request::ListAccounts{index, "", filterServer};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto owner = *local_nym_map_.at(2).rbegin();
    const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(command.FilterNotary(), filterServer);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::none);
    EXPECT_EQ(ids.size(), 0);
}

TEST_F(RPC_fixture, filter_unit_match)
{
    constexpr auto index{0};
    const auto filterUnit = created_units_.at(0);
    const auto command =
        ot::rpc::request::ListAccounts{index, "", "", filterUnit};
    const auto& list = command.asListAccounts();
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    // TODO const auto owner = *local_nym_map_.at(0).begin();
    // TODO const auto expected = [&]() -> std::set<std::string> {
    // TODO     return {registered_accounts_.at(owner).at(0)};
    // TODO }();
    // TODO const auto& ids = response.AccountIDs();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::list_accounts);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(command.FilterUnit(), filterUnit);
    EXPECT_EQ(list.AssociatedNyms().size(), 0);
    EXPECT_EQ(list.Cookie(), command.Cookie());
    EXPECT_EQ(list.Session(), command.Session());
    EXPECT_EQ(list.Type(), command.Type());
    EXPECT_EQ(list.Version(), command.Version());
    EXPECT_EQ(list.FilterNotary(), command.FilterNotary());
    EXPECT_EQ(list.FilterNym(), command.FilterNym());
    EXPECT_EQ(list.FilterUnit(), command.FilterUnit());
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
    // TODO EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::success);
    // TODO EXPECT_EQ(ids.size(), 1);

    // TODO for (const auto& id : ids) { EXPECT_EQ(expected.count(id), 1); }
}

TEST_F(RPC_fixture, cleanup) { Cleanup(); }
}  // namespace ottest
