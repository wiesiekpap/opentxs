// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/crypto/Element.hpp"
// IWYU pragma: no_include "opentxs/blockchain/block/bitcoin/Transaction.hpp"

#pragma once

#include <gtest/gtest.h>
#include <iterator>
#include <memory>
#include <regex>
#include <string>

#include "Basic.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/core/PasswordPrompt.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
class Manager;
}  // namespace client
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace crypto
{
class Element;
}  // namespace crypto
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionOutput;
}  // namespace proto

class Identifier;
class PasswordPrompt;
}  // namespace opentxs

namespace ottest
{
struct Test_BlockchainActivity : public ::testing::Test {
    using Element = ot::blockchain::crypto::Element;
    using Transaction = ot::blockchain::block::bitcoin::Transaction;

    static const std::string test_transaction_hex_;
    static const std::string btc_account_id_;
    static const std::string btc_unit_id_;
    static const std::string btc_notary_id_;
    static const std::string nym_1_name_;
    static const std::string nym_2_name_;
    static const std::string contact_3_name_;
    static const std::string contact_4_name_;
    static const std::string contact_5_name_;
    static const std::string contact_6_name_;
    static const std::string contact_7_name_;

    const ot::api::client::Manager& api_;
    const ot::OTPasswordPrompt reason_;

    auto account_1_id() const noexcept -> const ot::Identifier&;
    auto account_2_id() const noexcept -> const ot::Identifier&;
    auto contact_1_id() const noexcept -> const ot::Identifier&;
    auto contact_2_id() const noexcept -> const ot::Identifier&;
    auto contact_3_id() const noexcept -> const ot::Identifier&;
    auto contact_4_id() const noexcept -> const ot::Identifier&;
    auto contact_5_id() const noexcept -> const ot::Identifier&;
    auto contact_6_id() const noexcept -> const ot::Identifier&;
    auto contact_7_id() const noexcept -> const ot::Identifier&;
    auto get_test_transaction(
        const Element& first,
        const Element& second,
        const ot::Time& time = ot::Clock::now()) const
        -> std::unique_ptr<const Transaction>;
    auto get_test_transaction(
        const Element& inputKey1,
        const Element& inputKey2,
        const Element& inputKey3,
        const Element& outputKey1,
        const ot::proto::BlockchainTransactionOutput& prevOut1,
        const ot::proto::BlockchainTransactionOutput& prevOut2,
        const ot::proto::BlockchainTransactionOutput& prevOut3,
        const std::string& secondOutput,
        const ot::Time& time = ot::Clock::now()) const
        -> std::unique_ptr<const Transaction>;
    auto monkey_patch(const Element& first, const Element& second)
        const noexcept -> std::string;
    auto monkey_patch(const std::string& first, const std::string& second)
        const noexcept -> std::string;
    auto nym_1_id() const noexcept -> const ot::identifier::Nym&;
    auto nym_2_id() const noexcept -> const ot::identifier::Nym&;
    auto seed() const noexcept -> const std::string&;
    auto words() const noexcept -> const std::string&;

    Test_BlockchainActivity();
};
}  // namespace ottest
