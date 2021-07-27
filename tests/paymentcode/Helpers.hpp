// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "Basic.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/HDSeed.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/crypto/Asymmetric.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/crypto/PaymentCode.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client

class Core;
}  // namespace api
}  // namespace opentxs

namespace ottest
{
struct PaymentCodeFixture {
    static constexpr auto account_ = ot::Bip32Index{0};
    static constexpr auto index_ = ot::Bip32Index{0};

    auto blinding_key_public() -> const ot::crypto::key::EllipticCurve&;
    auto blinding_key_secret(
        const ot::api::client::Manager& api,
        const ot::blockchain::Type chain,
        const ot::PasswordPrompt& reason)
        -> const ot::crypto::key::EllipticCurve&;
    auto blinding_key_secret(
        const ot::api::client::Manager& api,
        const std::string& privateKey,
        const ot::PasswordPrompt& reason)
        -> const ot::crypto::key::EllipticCurve&;
    auto payment_code_public(
        const ot::api::Core& api,
        const std::string& base58) -> const ot::PaymentCode&;
    auto payment_code_secret(
        const ot::api::Core& api,
        const std::uint8_t version,
        const ot::PasswordPrompt& reason) -> const ot::PaymentCode&;
    auto seed(
        const ot::api::Core& api,
        const std::string_view wordList,
        const ot::PasswordPrompt& reason) -> const std::string&;

    auto cleanup() -> void;

    ~PaymentCodeFixture() { cleanup(); }

private:
    std::optional<std::string> seed_{};
    std::optional<ot::OTPaymentCode> pc_secret_{};
    std::optional<ot::OTPaymentCode> pc_public_{};
    std::unique_ptr<const ot::crypto::key::EllipticCurve> blind_key_secret_{};
    std::unique_ptr<const ot::crypto::key::EllipticCurve> blind_key_public_{};

    auto bip44_path(
        const ot::api::client::Manager& api,
        const ot::blockchain::Type chain,
        ot::AllocateOutput destination) const -> bool;
};

class PC_Fixture_Base : virtual public ::testing::Test
{
public:
    static PaymentCodeFixture user_1_;
    static PaymentCodeFixture user_2_;

    const ot::api::client::Manager& api_;
    const ot::OTPasswordPrompt reason_;
    const std::string& alice_seed_;
    const std::string& bob_seed_;
    const ot::PaymentCode& alice_pc_secret_;
    const ot::PaymentCode& alice_pc_public_;
    const ot::PaymentCode& bob_pc_secret_;
    const ot::PaymentCode& bob_pc_public_;

    virtual auto Shutdown() noexcept -> void;

    PC_Fixture_Base(
        const std::uint8_t aliceVersion,
        const std::uint8_t bobVersion,
        const std::string& aliceBip39,
        const std::string& bobBip39,
        const std::string& aliceExpectedPC,
        const std::string& bobExpectedPC) noexcept;
};
}  // namespace ottest
