// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <vector>

#include "Helpers.hpp"
#include "VectorsV1.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/crypto/AddressStyle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"

namespace ottest
{
class Test_PaymentCode_v1 : public PC_Fixture_Base
{
public:
    static constexpr auto version_ = std::uint8_t{1};
    static constexpr auto chain_ = ot::blockchain::Type::Bitcoin;

    const ot::crypto::key::EllipticCurve& alice_blind_secret_;
    const ot::crypto::key::EllipticCurve& alice_blind_public_;

    auto KeyToAddress(const ot::crypto::key::EllipticCurve& key) const noexcept
    {
        constexpr auto style = ot::blockchain::crypto::AddressStyle::P2PKH;

        return api_.Blockchain().CalculateAddress(
            chain_, style, api_.Factory().Data(key.PublicKey()));
    }

    Test_PaymentCode_v1()
        : PC_Fixture_Base(
              version_,
              version_,
              GetVectors1().alice_.words_,
              GetVectors1().bob_.words_,
              GetVectors1().alice_.payment_code_,
              GetVectors1().bob_.payment_code_)
        , alice_blind_secret_(user_1_.blinding_key_secret(
              api_,
              GetVectors1().alice_.private_key_,
              reason_))
        , alice_blind_public_(user_1_.blinding_key_public())
    {
    }
};

TEST_F(Test_PaymentCode_v1, generate)
{
    EXPECT_EQ(alice_pc_secret_.Version(), version_);
    EXPECT_EQ(alice_pc_public_.Version(), version_);
    EXPECT_EQ(bob_pc_secret_.Version(), version_);
    EXPECT_EQ(bob_pc_public_.Version(), version_);

    EXPECT_EQ(alice_pc_secret_.asBase58(), alice_pc_public_.asBase58());
    EXPECT_EQ(alice_pc_secret_.asBase58(), GetVectors1().alice_.payment_code_);
    EXPECT_EQ(bob_pc_secret_.asBase58(), bob_pc_public_.asBase58());
    EXPECT_EQ(bob_pc_secret_.asBase58(), GetVectors1().bob_.payment_code_);
}

TEST_F(Test_PaymentCode_v1, outgoing)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = alice_pc_secret_.Outgoing(
            bob_pc_public_, i, chain_, reason_, version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;

        EXPECT_EQ(
            KeyToAddress(key), GetVectors1().bob_.receiving_address_.at(i));
    }
}

TEST_F(Test_PaymentCode_v1, incoming)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = bob_pc_secret_.Incoming(
            alice_pc_public_, i, chain_, reason_, version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;

        EXPECT_EQ(
            KeyToAddress(key), GetVectors1().bob_.receiving_address_.at(i));
    }
}

TEST_F(Test_PaymentCode_v1, blind)
{
    const auto outpoint = api_.Factory().Data(
        GetVectors1().alice_.outpoint_, ot::StringStyle::Hex);
    const auto expected = api_.Factory().Data(
        GetVectors1().alice_.blinded_, ot::StringStyle::Hex);
    auto data = api_.Factory().Data();
    const auto blind = alice_pc_secret_.Blind(
        bob_pc_public_,
        alice_blind_secret_,
        outpoint->Bytes(),
        data->WriteInto(),
        reason_);

    ASSERT_TRUE(blind);
    EXPECT_EQ(data.get(), expected.get());
}

TEST_F(Test_PaymentCode_v1, unblind)
{
    const auto outpoint = api_.Factory().Data(
        GetVectors1().alice_.outpoint_, ot::StringStyle::Hex);
    const auto blinded = api_.Factory().Data(
        GetVectors1().alice_.blinded_, ot::StringStyle::Hex);
    const auto& expected = GetVectors1().alice_.payment_code_;
    auto data = api_.Factory().Data();
    const auto pPC = bob_pc_secret_.Unblind(
        blinded->Bytes(), alice_blind_public_, outpoint->Bytes(), reason_);

    ASSERT_TRUE(pPC);
    EXPECT_EQ(pPC->asBase58(), expected);
}

TEST_F(Test_PaymentCode_v1, shutdown) { Shutdown(); }
}  // namespace ottest
