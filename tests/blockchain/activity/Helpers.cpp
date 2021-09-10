// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/client/OTAPI_Exec.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/identity/Nym.hpp"

namespace ottest
{
const std::string Test_BlockchainActivity::test_transaction_hex_{
    "020000000001030f40268cb19feef75f1d748a8d0871756d1f1755f2c5820e2f02f9006959"
    "c878010000006a47304402203fff00984b87a0599810b7fbd0cde9d30c146dfa2667f2d9f4"
    "6847157439c22302203b848cd672f2ebe55058bb5cecd81aa7eecb896eedfdf834ad271559"
    "7d75b562012103d4a97baf62f4b17aa4bf011f2f1727988955297bd018431dce78dfd0a477"
    "b8f9ffffffff12ed50d95c48d9130f00c55cd3a7cc043df99730f416635d8f3852f01090e1"
    "af12000000171600149ce8880d3594c3397d58c47c539402b0f69c9992ffffffff1a335c3f"
    "5ca9aa1f9eae01c1e6365b9ad0073ab13c0e0b887b7adb6d524111bd1b0000006a47304402"
    "20308055ffee8665a904b7ce6e57296c69c343342ecc7f05683aad5d773cee23af02205aa3"
    "1ace19ea09f9994fe3349a843214c59b0c71d607aa627f7084225a9587e1012102b0c23712"
    "6a93c66fe86b878d7d4b616ea6bcb94006800e99b0085bbdb6d0562bffffffff02d20a0500"
    "000000001976a9144574e19db2911aa078671410f5e3bf502df2ae6f88ac8d071000000000"
    "001976a914cd6878fd84ce63a88104a1c8343bd63880b2989e88ac000247304402203dfc2c"
    "42e0a29c8a92e1a2df85360251aaa44c073effe4c96acedda605eeca5202206bc72a8a1ced"
    "e2ac16f5b7f5e0c2c9467279eea46fd04de115b4282d974dbd4e0121036963d16ea90bcd4c"
    "491dab7089faa5497c8179102c00750fc6eef40f02605d240000000000"};
const std::string Test_BlockchainActivity::btc_account_id_{
    "otw5eixq1554E4CZKgmxHqxusVDDd7m85VN"};
const std::string Test_BlockchainActivity::btc_unit_id_{
    "ot25c4KRyvcd9uiH6dYEsZXyXXbCzEuMcrtz"};
const std::string Test_BlockchainActivity::btc_notary_id_{
    "ot2BFCaEAASfsgspSq2KsZ1rqD5LvEN9BXvt"};
const std::string Test_BlockchainActivity::nym_1_name_{"Alex"};
const std::string Test_BlockchainActivity::nym_2_name_{"Bob"};
const std::string Test_BlockchainActivity::contact_3_name_{"Chris"};
const std::string Test_BlockchainActivity::contact_4_name_{"Daniel"};
const std::string Test_BlockchainActivity::contact_5_name_{"Edward"};
const std::string Test_BlockchainActivity::contact_6_name_{"Frank"};
const std::string Test_BlockchainActivity::contact_7_name_{"Gabe"};

Test_BlockchainActivity::Test_BlockchainActivity()
    : api_(ot::Context().StartClient(0))
    , reason_(api_.Factory().PasswordPrompt(__func__))
{
}

auto Test_BlockchainActivity::account_1_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Blockchain().NewHDSubaccount(
        nym_1_id(),
        ot::blockchain::crypto::HDProtocol::BIP_44,
        ot::blockchain::Type::Bitcoin,
        reason_);

    return output;
}

auto Test_BlockchainActivity::account_2_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Blockchain().NewHDSubaccount(
        nym_2_id(),
        ot::blockchain::crypto::HDProtocol::BIP_44,
        ot::blockchain::Type::Bitcoin,
        reason_);

    return output;
}

auto Test_BlockchainActivity::contact_1_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NymToContact(nym_1_id());

    return output;
}

auto Test_BlockchainActivity::contact_2_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NymToContact(nym_2_id());

    return output;
}

auto Test_BlockchainActivity::contact_3_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NewContactFromAddress(
        "1ANeKBrinuG86jw3rEvhFG6SYP1DCCzd4q",
        contact_3_name_,
        ot::blockchain::Type::Bitcoin);

    return output->ID();
}

auto Test_BlockchainActivity::contact_4_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NewContactFromAddress(
        "16C1f7wLAh44YgZ8oWJXaY9WoTPJzvfdqj",
        contact_4_name_,
        ot::blockchain::Type::Bitcoin);

    return output->ID();
}

auto Test_BlockchainActivity::contact_5_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NewContact(contact_5_name_);

    return output->ID();
}

auto Test_BlockchainActivity::contact_6_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NewContact(contact_6_name_);

    return output->ID();
}

auto Test_BlockchainActivity::contact_7_id() const noexcept
    -> const ot::Identifier&
{
    static const auto output = api_.Contacts().NewContactFromAddress(
        "17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem",
        contact_7_name_,
        ot::blockchain::Type::Bitcoin);

    return output->ID();
}

auto Test_BlockchainActivity::get_test_transaction(
    const Element& first,
    const Element& second,
    const ot::Time& time) const -> std::unique_ptr<const Transaction>
{
    const auto raw =
        api_.Factory().Data(monkey_patch(first, second), ot::StringStyle::Hex);
    auto output = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, raw->Bytes(), false, time);

    if (output) {
        auto& tx = dynamic_cast<
            ot::blockchain::block::bitcoin::internal::Transaction&>(
            const_cast<ot::blockchain::block::bitcoin::Transaction&>(*output));
        auto added = tx.ForTestingOnlyAddKey(0, first.KeyID());

        OT_ASSERT(added);

        added = tx.ForTestingOnlyAddKey(1, second.KeyID());

        OT_ASSERT(added);
    }

    return output;
}

auto Test_BlockchainActivity::get_test_transaction(
    const Element& inputKey1,
    const Element& inputKey2,
    const Element& inputKey3,
    const Element& outputKey1,
    const ot::proto::BlockchainTransactionOutput& prevOut1,
    const ot::proto::BlockchainTransactionOutput& prevOut2,
    const ot::proto::BlockchainTransactionOutput& prevOut3,
    const std::string& secondOutput,
    const ot::Time& time) const -> std::unique_ptr<const Transaction>
{
    const auto raw = api_.Factory().Data(
        monkey_patch(outputKey1.PubkeyHash()->asHex(), secondOutput),
        ot::StringStyle::Hex);
    auto output = api_.Factory().BitcoinTransaction(
        ot::blockchain::Type::Bitcoin, raw->Bytes(), false, time);

    if (output) {
        auto& tx = dynamic_cast<
            ot::blockchain::block::bitcoin::internal::Transaction&>(
            const_cast<ot::blockchain::block::bitcoin::Transaction&>(*output));
        auto added = tx.ForTestingOnlyAddKey(0, outputKey1.KeyID());

        OT_ASSERT(added);

        added = tx.AssociatePreviousOutput(api_.Blockchain(), 0, prevOut1);

        OT_ASSERT(added);

        added = tx.AssociatePreviousOutput(api_.Blockchain(), 1, prevOut2);

        OT_ASSERT(added);

        added = tx.AssociatePreviousOutput(api_.Blockchain(), 2, prevOut3);

        OT_ASSERT(added);
    }

    return output;
}

auto Test_BlockchainActivity::monkey_patch(
    const Element& first,
    const Element& second) const noexcept -> std::string
{
    return monkey_patch(
        first.PubkeyHash()->asHex(), second.PubkeyHash()->asHex());
}

auto Test_BlockchainActivity::monkey_patch(
    const std::string& first,
    const std::string& second) const noexcept -> std::string
{
    static const auto firstHash =
        std::regex{"4574e19db2911aa078671410f5e3bf502df2ae6f"};
    static const auto secondHash =
        std::regex{"cd6878fd84ce63a88104a1c8343bd63880b2989e"};
    static const auto input = std::string{test_transaction_hex_};

    OT_ASSERT(40 == first.size());
    OT_ASSERT(40 == second.size());

    auto output = std::string{};
    auto temp = std::string{};
    std::regex_replace(
        std::back_inserter(temp), input.begin(), input.end(), firstHash, first);
    std::regex_replace(
        std::back_inserter(output),
        temp.begin(),
        temp.end(),
        secondHash,
        second);

    return output;
}

auto Test_BlockchainActivity::nym_1_id() const noexcept
    -> const ot::identifier::Nym&
{
    static const auto output =
        api_.Wallet().Nym(reason_, nym_1_name_, {seed(), 0});

    return output->ID();
}

auto Test_BlockchainActivity::nym_2_id() const noexcept
    -> const ot::identifier::Nym&
{
    static const auto output =
        api_.Wallet().Nym(reason_, nym_2_name_, {seed(), 1});

    return output->ID();
}

auto Test_BlockchainActivity::seed() const noexcept -> const std::string&
{
    static const auto output = api_.Exec().Wallet_ImportSeed(words(), "");

    return output;
}

auto Test_BlockchainActivity::words() const noexcept -> const std::string&
{
    static const auto output =
        std::string{"response seminar brave tip suit recall often sound "
                    "stick owner lottery motion"};

    return output;
}
}  // namespace ottest
