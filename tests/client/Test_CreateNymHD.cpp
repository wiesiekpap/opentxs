// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "internal/api/session/Client.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Numbers.hpp"

using namespace opentxs;

namespace ot = opentxs;

namespace ottest
{
class Test_CreateNymHD : public ::testing::Test
{
public:
    const ot::api::session::Client& client_;
    ot::OTPasswordPrompt reason_;
    std::string SeedA_;
    std::string SeedB_;
    std::string SeedC_;
    std::string SeedD_;
    std::string AliceID, BobID, EveID, FrankID;
    std::string Alice, Bob;

    Test_CreateNymHD()
        : client_(ot::Context().StartClientSession(0))
        , reason_(client_.Factory().PasswordPrompt(__func__))
        // these fingerprints are deterministic so we can share them among tests
        , SeedA_(client_.InternalClient().Exec().Wallet_ImportSeed(
              "spike nominee miss inquiry fee nothing belt list other daughter "
              "leave valley twelve gossip paper",
              ""))
        , SeedB_(client_.InternalClient().Exec().Wallet_ImportSeed(
              "glimpse destroy nation advice seven useless candy move number "
              "toast insane anxiety proof enjoy lumber",
              ""))
        , SeedC_(client_.InternalClient().Exec().Wallet_ImportSeed(
              "park cabbage quit",
              ""))
        , SeedD_(client_.InternalClient().Exec().Wallet_ImportSeed(
              "federal dilemma rare",
              ""))
        , AliceID("ot24XFA1wKynjaAB59dx7PwEzGg37U8Q2yXG")
        , BobID("ot274uRuN1VezD47R7SqAH27s2WKP1U5jKWk")
        , EveID("otwz4jCuiVg7UF2i1NgCSvTWeDS29EAHeL6")
        , FrankID("otogQecfWnoJn5Juy5z5Si3mS53rTb7LgGe")
        , Alice(client_.Wallet()
                    .Nym({SeedA_, 0, 1}, reason_, "Alice")
                    ->ID()
                    .str())
        , Bob(client_.Wallet().Nym({SeedB_, 0, 1}, reason_, "Bob")->ID().str())
    {
    }
};

TEST_F(Test_CreateNymHD, TestNym_DeterministicIDs)
{

    EXPECT_STREQ(AliceID.c_str(), Alice.c_str());
    EXPECT_STREQ(BobID.c_str(), Bob.c_str());
}

TEST_F(Test_CreateNymHD, TestNym_ABCD)
{
    const auto NymA = client_.Wallet().Nym(identifier::Nym::Factory(Alice));
    const auto NymB = client_.Wallet().Nym(identifier::Nym::Factory(Bob));
    const auto NymC = client_.Wallet().Nym({SeedA_, 1}, reason_, "Charly");
    const auto NymD = client_.Wallet().Nym({SeedB_, 1}, reason_, "Dave");

    // Alice
    EXPECT_TRUE(NymA->HasPath());
    EXPECT_STREQ(NymA->PathRoot().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, NymA->PathChildSize());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        NymA->PathChild(0));

    EXPECT_EQ(
        0 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymA->PathChild(1));

    // Bob
    EXPECT_TRUE(NymB->HasPath());
    EXPECT_STREQ(NymB->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, NymB->PathChildSize());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        NymB->PathChild(0));

    EXPECT_EQ(
        0 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymB->PathChild(1));

    // Charly
    EXPECT_TRUE(NymC->HasPath());
    EXPECT_STREQ(NymC->PathRoot().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, NymC->PathChildSize());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        NymC->PathChild(0));

    EXPECT_EQ(
        1 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymC->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_Dave)
{
    const auto NymD = client_.Wallet().Nym({SeedB_, 1}, reason_, "Dave");

    ASSERT_TRUE(NymD);

    EXPECT_TRUE(NymD->HasPath());
    EXPECT_STREQ(NymD->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, NymD->PathChildSize());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        NymD->PathChild(0));

    EXPECT_EQ(
        1 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymD->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_Eve)
{
    const auto NymE = client_.Wallet().Nym({SeedB_, 2, 1}, reason_, "Eve");

    ASSERT_TRUE(NymE);

    EXPECT_EQ(EveID, NymE->ID().str());

    EXPECT_TRUE(NymE->HasPath());
    EXPECT_STREQ(NymE->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, NymE->PathChildSize());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        NymE->PathChild(0));

    EXPECT_EQ(
        2 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymE->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_Frank)
{
    const auto NymF = client_.Wallet().Nym({SeedB_, 3, 3}, reason_, "Frank");
    const auto NymF2 = client_.Wallet().Nym({SeedA_, 3, 3}, reason_, "Frank");

    EXPECT_NE(NymF->ID(), NymF2->ID());

    EXPECT_EQ(FrankID, NymF->ID().str());

    EXPECT_TRUE(NymF->HasPath());
    EXPECT_TRUE(NymF2->HasPath());

    EXPECT_STREQ(NymF->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_STREQ(NymF2->PathRoot().c_str(), SeedA_.c_str());

    EXPECT_EQ(2, NymF->PathChildSize());
    EXPECT_EQ(2, NymF2->PathChildSize());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        NymF->PathChild(0));

    EXPECT_EQ(
        3 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymF->PathChild(1));
    EXPECT_EQ(
        3 | static_cast<Bip32Index>(Bip32Child::HARDENED), NymF2->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_NonnegativeIndex)
{
    const auto Nym1 = client_.Wallet().Nym({SeedC_, 0}, reason_, "Nym1");
    const auto Nym2 = client_.Wallet().Nym({SeedC_, 0}, reason_, "Nym2");

    EXPECT_TRUE(Nym1->HasPath());
    EXPECT_TRUE(Nym2->HasPath());

    const auto nym1Index = Nym1->PathChild(1);
    const auto nym2Index = Nym2->PathChild(1);

    ASSERT_EQ(nym1Index, nym2Index);
}

TEST_F(Test_CreateNymHD, TestNym_NegativeIndex)
{
    const auto Nym1 = client_.Wallet().Nym({SeedD_, -1}, reason_, "Nym1");
    const auto Nym2 = client_.Wallet().Nym({SeedD_, -1}, reason_, "Nym2");

    EXPECT_TRUE(Nym1->HasPath());
    EXPECT_TRUE(Nym2->HasPath());

    const auto nym1Index = Nym1->PathChild(1);
    const auto nym2Index = Nym2->PathChild(1);

    ASSERT_NE(nym1Index, nym2Index);
}
}  // namespace ottest
