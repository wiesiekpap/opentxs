// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <future>

#include "opentxs/Pimpl.hpp"
#include "opentxs/Shared.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "paymentcode/VectorsV3.hpp"

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
        session.OTX().DisableAutoaccept();
        session.Pair().Stop().get();
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
            RegisterAccount(session, server, chris, unit, "chris's dollars");

        EXPECT_FALSE(account1.empty());
        EXPECT_FALSE(account2.empty());
        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 3);
        EXPECT_EQ(created_units_.size(), 1);
    }
}

TEST_F(RPC_fixture, bad_client_session)
{
    constexpr auto index{88};
    const auto& api = ot_.Client(0);
    const auto& source = registered_accounts_.at(issuer_).front();
    const auto& destination = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(brian_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, source, destination, recipient->str(), 100};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
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
}

TEST_F(RPC_fixture, bad_server_session)
{
    constexpr auto index{99};
    const auto& api = ot_.Client(0);
    const auto& source = registered_accounts_.at(issuer_).front();
    const auto& destination = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(brian_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, source, destination, recipient->str(), 100};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
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
}

TEST_F(RPC_fixture, transfer)
{
    constexpr auto index{0};
    const auto& api = ot_.Client(0);
    const auto& server = ot_.Server(0);
    const auto& source = registered_accounts_.at(issuer_).front();
    const auto& destination = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(brian_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, source, recipient->str(), destination, 100};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::queued);
    ASSERT_TRUE(push_.wait(0));
    // TODO validate each push message

    RefreshAccount(api, api.Factory().NymID(brian_), server.ID());
    RefreshAccount(api, api.Factory().NymID(issuer_), server.ID());

    ASSERT_TRUE(push_.wait(2));
    // TODO validate each push message
}

TEST_F(RPC_fixture, cheque)
{
    constexpr auto index{0};
    const auto& api = ot_.Client(0);
    const auto& server = ot_.Server(0);
    constexpr auto type = ot::rpc::PaymentType::cheque;
    const auto& source = registered_accounts_.at(brian_).front();
    const auto recipient =
        api.Contacts().ContactID(api.Factory().NymID(chris_));

    ASSERT_FALSE(recipient->empty());

    const auto command = ot::rpc::request::SendPayment{
        index, type, source, recipient->str(), 75};
    const auto& list = command.asSendPayment();
    const auto base = ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), rpc::CommandType::send_payment);
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
    EXPECT_EQ(codes.at(0).second, rpc::ResponseCode::queued);
    ASSERT_TRUE(push_.wait(3));
    // TODO validate each push message

    RefreshAccount(api, api.Factory().NymID(chris_), server.ID());

    ASSERT_TRUE(push_.wait(4));

    EXPECT_EQ(DepositCheques(api, server, chris_), 1);

    RefreshAccount(api, api.Factory().NymID(brian_), server.ID());

    ASSERT_TRUE(push_.wait(1));
    // TODO validate each push message
}

TEST_F(RPC_fixture, postconditions)
{
    const auto& api = ot_.Client(0);
    auto account1 = api.Wallet().Account(
        api.Factory().Identifier(registered_accounts_.at(issuer_).front()));
    auto account2 = api.Wallet().Account(
        api.Factory().Identifier(registered_accounts_.at(brian_).front()));
    auto account3 = api.Wallet().Account(
        api.Factory().Identifier(registered_accounts_.at(chris_).front()));

    EXPECT_EQ(account1.get().GetBalance(), -100);
    EXPECT_EQ(account2.get().GetBalance(), 25);
    EXPECT_EQ(account3.get().GetBalance(), 75);
}

// TODO test other instruments
// TODO add test cases for all failure modes

TEST_F(RPC_fixture, cleanup) { Cleanup(); }
}  // namespace ottest
