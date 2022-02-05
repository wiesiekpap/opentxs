// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>

#include "Basic.hpp"
#include "integration/Helpers.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/otx/client/Pair.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/interface/qt/AccountActivity.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "rpc/Helpers.hpp"

namespace ot = opentxs;

namespace ottest
{
constexpr auto issuer_{
    "ot2xuVPJDdweZvKLQD42UMCzhCmT3okn3W1PktLgCbmQLRnaKy848sX"};

class Test_AmountValidator : public RPC_fixture
{
public:
    struct ChainAmounts {
        ot::UnitType unittype_;
        std::vector<std::pair<ot::UnallocatedCString, ot::UnallocatedCString>>
            amounts_;
        std::vector<std::pair<ot::UnallocatedCString, ot::UnallocatedCString>>
            invalid_amounts_;
    };
};

TEST_F(Test_AmountValidator, preconditions)
{
    {
        const auto& session = StartNotarySession(0);
        const auto instance = session.Instance();
        const auto& seeds = seed_map_.at(instance);
        const auto& nyms = local_nym_map_.at(instance);

        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
    }
    {
        const auto& server = ot_.NotarySession(0);
        const auto& session = StartClient(0);
        session.OTX().DisableAutoaccept();
        session.InternalClient().Pair().Stop().get();
        const auto seed = ImportBip39(session, GetVectors3().alice_.words_);

        EXPECT_FALSE(seed.empty());
        EXPECT_TRUE(SetIntroductionServer(session, server));

        const auto& issuer = CreateNym(session, "issuer", seed, 0);

        EXPECT_EQ(issuer.nym_id_->str(), issuer_);
        EXPECT_TRUE(RegisterNym(server, issuer));
    }
}

TEST_F(Test_AmountValidator, decimals)
{
    const auto& server = ot_.NotarySession(0);
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto& def = ot::display::GetDefinition(ot::UnitType::Regtest);
    const auto unit = IssueUnit(
        server,
        issuer,
        "decimals-" + ot::UnallocatedCString{def.ShortName()},
        "Test",
        ot::UnitType::Regtest,
        def);

    EXPECT_FALSE(unit.empty());

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()].back()));

    auto& amountValidator = *activity->getAmountValidator();
    amountValidator.setScale(0);

    auto inputString = QString{"1.234567890"};
    auto pos = 0;
    auto validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 units");

    inputString = QString{"1.234567890"};
    amountValidator.setMaxDecimals(4);

    EXPECT_EQ(amountValidator.getMaxDecimals(), 4);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F6 units");

    inputString = QString{"1.2"};
    amountValidator.setMinDecimals(3);

    EXPECT_EQ(amountValidator.getMinDecimals(), 3);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.200 units");

    inputString = QString{"1"};
    amountValidator.setMaxDecimals(8);
    amountValidator.setMinDecimals(5);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Acceptable);
    EXPECT_EQ(inputString.toStdString(), "1.000\u202F00 units");
}

TEST_F(Test_AmountValidator, fixup)
{
    const auto& server = ot_.NotarySession(0);
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto& def = ot::display::GetDefinition(ot::UnitType::Regtest);
    const auto unit = IssueUnit(
        server,
        issuer,
        "fixup-" + ot::UnallocatedCString{def.ShortName()},
        "Test",
        ot::UnitType::Regtest,
        def);

    EXPECT_FALSE(unit.empty());

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()].back()));

    auto& amountValidator = *activity->getAmountValidator();

    auto inputString = QString{"1.234567890"};
    amountValidator.fixup(inputString);

    // fixup doesn't do anything
    EXPECT_EQ(inputString.toStdString(), "1.234567890");
}

TEST_F(Test_AmountValidator, revise)
{
    const auto& server = ot_.NotarySession(0);
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto& def = ot::display::GetDefinition(ot::UnitType::Bch);
    const auto unit = IssueUnit(
        server,
        issuer,
        "revise-" + ot::UnallocatedCString{def.ShortName()},
        "Test",
        ot::UnitType::Bch,
        def);

    EXPECT_FALSE(unit.empty());

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()].back()));

    auto& amountValidator = *activity->getAmountValidator();
    amountValidator.setScale(0);

    auto inputString = QString{"1.234567890"};
    auto pos = 0;
    auto validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 BCH");

    amountValidator.setScale(1);
    auto revised = amountValidator.revise(inputString, 0);
    EXPECT_EQ(revised.toStdString(), "1,234.567\u202F89 mBCH");

    amountValidator.setScale(0);
    revised = amountValidator.revise(revised, 1);
    EXPECT_EQ(revised.toStdString(), "1.234\u202F567\u202F89 BCH");

    amountValidator.setScale(2);
    revised = amountValidator.revise(revised, 0);
    EXPECT_EQ(revised.toStdString(), "1,234,567.89 bits");

    amountValidator.setScale(3);
    revised = amountValidator.revise(revised, 2);
    EXPECT_EQ(revised.toStdString(), "1,234,567.89 μBCH");

    amountValidator.setScale(4);
    revised = amountValidator.revise(revised, 3);
    EXPECT_EQ(revised.toStdString(), "123,456,789 satoshis");
}

TEST_F(Test_AmountValidator, scale)
{
    const auto& server = ot_.NotarySession(0);
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    const auto& def = ot::display::GetDefinition(ot::UnitType::Bch);
    const auto unit = IssueUnit(
        server,
        issuer,
        "scale-" + ot::UnallocatedCString{def.ShortName()},
        "Test",
        ot::UnitType::Bch,
        def);

    EXPECT_FALSE(unit.empty());

    const auto* activity = session.UI().AccountActivityQt(
        issuer.nym_id_,
        ot::Identifier::Factory(
            registered_accounts_[issuer.nym_id_->str()].back()));

    auto& amountValidator = *activity->getAmountValidator();
    amountValidator.setScale(0);

    auto inputString = QString{"1.234567890"};
    auto pos = 0;
    auto validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F567\u202F89 BCH");

    inputString = QString{"1.234567890"};
    amountValidator.setScale(1);

    EXPECT_EQ(amountValidator.getScale(), 1);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.234\u202F57 mBCH");

    inputString = QString{"1.234567890"};
    amountValidator.setScale(2);

    EXPECT_EQ(amountValidator.getScale(), 2);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.23 bits");

    inputString = QString{"1.234567890"};
    amountValidator.setScale(3);

    EXPECT_EQ(amountValidator.getScale(), 3);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1.23 μBCH");

    inputString = QString{"1.234567890"};
    amountValidator.setScale(4);

    EXPECT_EQ(amountValidator.getScale(), 4);

    validated = amountValidator.validate(inputString, pos);

    EXPECT_EQ(validated, QValidator::State::Intermediate);
    EXPECT_EQ(inputString.toStdString(), "1 satoshis");
}

TEST_F(Test_AmountValidator, validate)
{
    static const auto chains = std::vector<ChainAmounts>{
        {ot::UnitType::Btc,
         {{u8"0", u8"0 ₿"},
          {u8"1", u8"1 ₿"},
          {u8"10", u8"10 ₿"},
          {u8"25", u8"25 ₿"},
          {u8"300", u8"300 ₿"},
          {u8"4567", u8"4,567 ₿"},
          {u8"50000", u8"50,000 ₿"},
          {u8"678901", u8"678,901 ₿"},
          {u8"7000000", u8"7,000,000 ₿"},
          {u8"1000000000000000001", u8"1,000,000,000,000,000,001 ₿"},
          {u8".1", u8"0.1 ₿"},
          {u8".123456789", u8"0.123\u202F456\u202F79 ₿"},
          {u8"74.99999448", u8"74.999\u202F994\u202F48 ₿"},
          {u8"86.00002652", u8"86.000\u202F026\u202F52 ₿"},
          {u8"89.99999684", u8"89.999\u202F996\u202F84 ₿"},
          {u8"100.0000495", u8"100.000\u202F049\u202F5 ₿"}},
         {
             {u8"100000000000000000000000000000000000000"
              u8"000000000000000000000000000000000000000",
              u8"100000000000000000000000000000000000000"
              u8"000000000000000000000000000000000000000 ₿"},
             {u8"1.2.3", u8"1.23 ₿"},
         }},
        {ot::UnitType::Ltc,
         {{u8"0", u8"0 Ł"},
          {u8"1", u8"1 Ł"},
          {u8"10", u8"10 Ł"},
          {u8"25", u8"25 Ł"},
          {u8"300", u8"300 Ł"},
          {u8"4567", u8"4,567 Ł"},
          {u8"50000", u8"50,000 Ł"},
          {u8"678901", u8"678,901 Ł"},
          {u8"7000000", u8"7,000,000 Ł"},
          {u8"1000000000000000001", u8"1,000,000,000,000,000,001 Ł"},
          {u8".1", u8"0.1 Ł"},
          {u8".123456789", u8"0.123\u202F457 Ł"},
          {u8"74.99999448", u8"74.999\u202F994 Ł"},
          {u8"86.00002652", u8"86.000\u202F027 Ł"},
          {u8"89.99999684", u8"89.999\u202F997 Ł"},
          {u8"100.0000495", u8"100.000\u202F05 Ł"}},
         {
             {u8"100000000000000000000000000000000000000"
              u8"000000000000000000000000000000000000000",
              u8"100000000000000000000000000000000000000"
              u8"000000000000000000000000000000000000000 Ł"},
             {u8"1.2.3", u8"1.23 Ł"},
         }},
        {ot::UnitType::Pkt,
         {{u8"0", u8"0 PKT"},
          {u8"1", u8"1 PKT"},
          {u8"10", u8"10 PKT"},
          {u8"25", u8"25 PKT"},
          {u8"300", u8"300 PKT"},
          {u8"4567", u8"4,567 PKT"},
          {u8"50000", u8"50,000 PKT"},
          {u8"678901", u8"678,901 PKT"},
          {u8"7000000", u8"7,000,000 PKT"},
          {u8"1000000000000000001", u8"1,000,000,000,000,000,001 PKT"},
          {u8".1", u8"0.1 PKT"},
          {u8".123456789012", u8"0.123\u202F456\u202F789\u202F01 PKT"},
          {u8"74.99999448", u8"74.999\u202F994\u202F48 PKT"},
          {u8"86.00002652", u8"86.000\u202F026\u202F52 PKT"},
          {u8"89.99999684", u8"89.999\u202F996\u202F84 PKT"},
          {u8"100.0000495", u8"100.000\u202F049\u202F5 PKT"}},
         {
             {u8"100000000000000000000000000000000000000"
              u8"000000000000000000000000000000000000000",
              u8"100000000000000000000000000000000000000"
              u8"000000000000000000000000000000000000000 PKT"},
             {u8"1.2.3", u8"1.23 PKT"},
         }},
        {ot::UnitType::Usd,
         {{u8"0", u8"$0.00"},
          {u8"1", u8"$1.00"},
          {u8"10", u8"$10.00"},
          {u8"25", u8"$25.00"},
          {u8"300", u8"$300.00"},
          {u8"4567", u8"$4,567.00"},
          {u8"50000", u8"$50,000.00"},
          {u8"678901", u8"$678,901.00"},
          {u8"7000000", u8"$7,000,000.00"},
          {u8"1000000000000000001", u8"$1,000,000,000,000,000,001.00"},
          {u8"115792089237316195423570985008687907853"
           u8"269984665640564039457584007913129639935",
           u8"$115,792,089,237,316,195,423,570,985,008,687,907,853,"
           u8"269,984,665,640,564,039,457,584,007,913,129,639,935.00"},
          {u8".1", u8"$0.10"},
          {u8".1234", u8"$0.123"},
          {u8"74.99999448", u8"$75.00"},
          {u8"86.00002652", u8"$86.00"},
          {u8"89.99999684", u8"$90.00"},
          {u8"100.0000495", u8"$100.00"}},
         {
             {u8"115792089237316195423570985008687907853"
              u8"269984665640564039457584007913129639936",
              u8"$115,792,089,237,316,195,423,570,985,008,687,907,853,"
              u8"269,984,665,640,564,039,457,584,007,913,129,639,936.00"},
             {u8"100000000000000000000000000000000000000"
              u8"0000000000000000000000000000000000000000",
              u8"$100000000000000000000000000000000000000"
              u8"0000000000000000000000000000000000000000.00"},
             {u8"1.2.3", u8"$1.00"},
         }},
    };

    const auto& server = ot_.NotarySession(0);
    const auto& session = ot_.ClientSession(0);

    const auto& issuer = users_.at(0);

    for (const auto& chain : chains) {
        const auto& def = ot::display::GetDefinition(chain.unittype_);
        const auto unit = IssueUnit(
            server,
            issuer,
            "validate-" + ot::UnallocatedCString{def.ShortName()},
            "Test",
            chain.unittype_,
            def);
        EXPECT_FALSE(unit.empty());

        const auto* activity = session.UI().AccountActivityQt(
            issuer.nym_id_,
            opentxs::Identifier::Factory(
                registered_accounts_[issuer.nym_id_->str()].back()));

        const auto& amountValidator = *activity->getAmountValidator();

        auto pos = 0;
        for (const auto& [input, valid] : chain.amounts_) {
            auto inputString = QString{input.c_str()};
            const auto validated = amountValidator.validate(inputString, pos);

            EXPECT_EQ(validated, QValidator::State::Acceptable);
            EXPECT_EQ(inputString.toStdString(), valid);
        }

        for (const auto& [input, valid] : chain.invalid_amounts_) {
            auto inputString = QString{input.c_str()};
            const auto validated = amountValidator.validate(inputString, pos);

            EXPECT_EQ(validated, QValidator::State::Invalid);
            EXPECT_NE(inputString.toStdString(), valid);
        }
    }
}

TEST_F(Test_AmountValidator, cleanup) { Cleanup(); }
}  // namespace ottest
