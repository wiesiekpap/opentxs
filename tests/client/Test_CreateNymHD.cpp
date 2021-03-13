// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <string>

#include "OTTestEnvironment.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/protobuf/HDPath.pb.h"

using namespace opentxs;

namespace
{
#if OT_CRYPTO_WITH_BIP32
class Test_CreateNymHD : public ::testing::Test
{
public:
    const ot::api::client::Manager& client_;
    ot::OTPasswordPrompt reason_;
    std::string SeedA_;
    std::string SeedB_;
    std::string SeedC_;
    std::string SeedD_;
    std::string AliceID, BobID, EveID, FrankID;
    std::string Alice, Bob;

    Test_CreateNymHD()
        : client_(ot::Context().StartClient({}, 0))
        , reason_(client_.Factory().PasswordPrompt(__FUNCTION__))
        // these fingerprints are deterministic so we can share them among tests
        , SeedA_(client_.Exec().Wallet_ImportSeed(
              "spike nominee miss inquiry fee nothing belt list other daughter "
              "leave valley twelve gossip paper",
              ""))
        , SeedB_(client_.Exec().Wallet_ImportSeed(
              "glimpse destroy nation advice seven useless candy move number "
              "toast insane anxiety proof enjoy lumber",
              ""))
        , SeedC_(client_.Exec().Wallet_ImportSeed("park cabbage quit", ""))
        , SeedD_(client_.Exec().Wallet_ImportSeed("federal dilemma rare", ""))
        , AliceID("ot24XFA1wKynjaAB59dx7PwEzGg37U8Q2yXG")
        , BobID("ot274uRuN1VezD47R7SqAH27s2WKP1U5jKWk")
        , EveID("otwz4jCuiVg7UF2i1NgCSvTWeDS29EAHeL6")
        , FrankID("otogQecfWnoJn5Juy5z5Si3mS53rTb7LgGe")
        , Alice(client_.Wallet()
                    .Nym(reason_, "Alice", {SeedA_, 0, 1})
                    ->ID()
                    .str())
        , Bob(client_.Wallet().Nym(reason_, "Bob", {SeedB_, 0, 1})->ID().str())
    {
    }
};

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
TEST_F(Test_CreateNymHD, TestNym_DeterministicIDs)
{

    EXPECT_STREQ(AliceID.c_str(), Alice.c_str());
    EXPECT_STREQ(BobID.c_str(), Bob.c_str());
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

TEST_F(Test_CreateNymHD, TestNym_ABCD)
{
    const auto NymA = client_.Wallet().Nym(identifier::Nym::Factory(Alice));
    const auto NymB = client_.Wallet().Nym(identifier::Nym::Factory(Bob));
    const auto NymC = client_.Wallet().Nym(reason_, "Charly", {SeedA_, 1});
    const auto NymD = client_.Wallet().Nym(reason_, "Dave", {SeedB_, 1});

    // Alice
    proto::HDPath pathA;
    EXPECT_TRUE(NymA->Path(pathA));
    EXPECT_STREQ(pathA.root().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, pathA.child_size());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        pathA.child(0));

    EXPECT_EQ(
        0 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathA.child(1));

    // Bob
    proto::HDPath pathB;
    EXPECT_TRUE(NymB->Path(pathB));
    EXPECT_STREQ(pathB.root().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, pathB.child_size());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        pathB.child(0));

    EXPECT_EQ(
        0 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathB.child(1));

    // Charly
    proto::HDPath pathC;
    EXPECT_TRUE(NymC->Path(pathC));
    EXPECT_STREQ(pathC.root().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, pathC.child_size());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        pathC.child(0));

    EXPECT_EQ(
        1 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathC.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_Dave)
{
    const auto NymD = client_.Wallet().Nym(reason_, "Dave", {SeedB_, 1});

    ASSERT_TRUE(NymD);

    proto::HDPath pathD;
    EXPECT_TRUE(NymD->Path(pathD));
    EXPECT_STREQ(pathD.root().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, pathD.child_size());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        pathD.child(0));

    EXPECT_EQ(
        1 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathD.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_Eve)
{
    const auto NymE = client_.Wallet().Nym(reason_, "Eve", {SeedB_, 2, 1});

    ASSERT_TRUE(NymE);

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    EXPECT_EQ(EveID, NymE->ID().str());
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    proto::HDPath pathE;
    EXPECT_TRUE(NymE->Path(pathE));
    EXPECT_STREQ(pathE.root().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, pathE.child_size());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        pathE.child(0));

    EXPECT_EQ(
        2 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathE.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_Frank)
{
    const auto NymF = client_.Wallet().Nym(reason_, "Frank", {SeedB_, 3, 3});
    const auto NymF2 = client_.Wallet().Nym(reason_, "Frank", {SeedA_, 3, 3});

    EXPECT_NE(NymF->ID(), NymF2->ID());

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32
    EXPECT_EQ(FrankID, NymF->ID().str());
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1 && OT_CRYPTO_WITH_BIP32

    proto::HDPath pathF, pathF2;
    EXPECT_TRUE(NymF->Path(pathF));
    EXPECT_TRUE(NymF2->Path(pathF2));

    EXPECT_STREQ(pathF.root().c_str(), SeedB_.c_str());
    EXPECT_STREQ(pathF2.root().c_str(), SeedA_.c_str());

    EXPECT_EQ(2, pathF.child_size());
    EXPECT_EQ(2, pathF2.child_size());

    EXPECT_EQ(
        static_cast<Bip32Index>(Bip43Purpose::NYM) |
            static_cast<Bip32Index>(Bip32Child::HARDENED),
        pathF.child(0));

    EXPECT_EQ(
        3 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathF.child(1));
    EXPECT_EQ(
        3 | static_cast<Bip32Index>(Bip32Child::HARDENED), pathF2.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_NonnegativeIndex)
{
    const auto Nym1 = client_.Wallet().Nym(reason_, "Nym1", {SeedC_, 0});
    const auto Nym2 = client_.Wallet().Nym(reason_, "Nym2", {SeedC_, 0});

    proto::HDPath path1, path2;
    EXPECT_TRUE(Nym1->Path(path1));
    EXPECT_TRUE(Nym2->Path(path2));

    const auto nym1Index = path1.child(1);
    const auto nym2Index = path2.child(1);

    ASSERT_EQ(nym1Index, nym2Index);
}

TEST_F(Test_CreateNymHD, TestNym_NegativeIndex)
{
    const auto Nym1 = client_.Wallet().Nym(reason_, "Nym1", {SeedD_, -1});
    const auto Nym2 = client_.Wallet().Nym(reason_, "Nym2", {SeedD_, -1});

    proto::HDPath path1, path2;
    EXPECT_TRUE(Nym1->Path(path1));
    EXPECT_TRUE(Nym2->Path(path2));

    const auto nym1Index = path1.child(1);
    const auto nym2Index = path2.child(1);

    ASSERT_NE(nym1Index, nym2Index);
}
#endif  // OT_CRYPTO_WITH_BIP32
}  // namespace
