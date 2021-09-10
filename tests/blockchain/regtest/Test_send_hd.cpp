// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <deque>
#include <optional>
#include <set>
#include <string>
#include <utility>

#include "opentxs/Pimpl.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/rpc/AccountData.hpp"
#include "opentxs/rpc/AccountEvent.hpp"
#include "opentxs/rpc/AccountEventType.hpp"
#include "opentxs/rpc/AccountType.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/GetAccountActivity.hpp"
#include "opentxs/rpc/request/GetAccountBalance.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/GetAccountActivity.hpp"
#include "opentxs/rpc/response/GetAccountBalance.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
Counter account_list_{};
Counter account_activity_{};

class Regtest_fixture_hd : public Regtest_fixture_normal
{
protected:
    using Subchain = ot::blockchain::crypto::Subchain;
    using UTXO = ot::blockchain::node::Wallet::UTXO;

    static ot::Nym_p alex_p_;
    static std::deque<ot::blockchain::block::pTxid> transactions_;
    static std::unique_ptr<ScanListener> listener_p_;
    static Expected expected_;

    const ot::identity::Nym& alex_;
    const ot::blockchain::crypto::HD& account_;
    const ot::Identifier& expected_account_;
    const ot::identifier::Server& expected_notary_;
    const ot::identifier::UnitDefinition& expected_unit_;
    const std::string expected_display_unit_;
    const std::string expected_account_name_;
    const std::string expected_notary_name_;
    const std::string memo_outgoing_;
    const ot::AccountType expected_account_type_;
    const ot::contact::ContactItemType expected_unit_type_;
    const Generator hd_generator_;
    ScanListener& listener_;

    auto Shutdown() noexcept -> void final
    {
        listener_p_.reset();
        transactions_.clear();
        alex_p_.reset();
        Regtest_fixture_normal::Shutdown();
    }

    Regtest_fixture_hd()
        : Regtest_fixture_normal(1)
        , alex_([&]() -> const ot::identity::Nym& {
            if (!alex_p_) {
                const auto reason =
                    client_1_.Factory().PasswordPrompt(__func__);

                alex_p_ = client_1_.Wallet().Nym(reason, "Alex");

                OT_ASSERT(alex_p_)

                client_1_.Blockchain().NewHDSubaccount(
                    alex_p_->ID(),
                    ot::blockchain::crypto::HDProtocol::BIP_44,
                    test_chain_,
                    reason);
            }

            OT_ASSERT(alex_p_)

            return *alex_p_;
        }())
        , account_(client_1_.Blockchain()
                       .Account(alex_.ID(), test_chain_)
                       .GetHD()
                       .at(0))
        , expected_account_(account_.Parent().AccountID())
        , expected_notary_(client_1_.UI().BlockchainNotaryID(test_chain_))
        , expected_unit_(client_1_.UI().BlockchainUnitID(test_chain_))
        , expected_display_unit_(u8"UNITTEST")
        , expected_account_name_(u8"This device")
        , expected_notary_name_(u8"Unit Test Simulation")
        , memo_outgoing_("memo for outgoing transaction")
        , expected_account_type_(ot::AccountType::Blockchain)
        , expected_unit_type_(ot::contact::ContactItemType::Regtest)
        , hd_generator_([&](Height height) -> Transaction {
            using OutputBuilder = ot::api::Factory::OutputBuilder;
            using Index = ot::Bip32Index;
            static constexpr auto count = 100u;
            static constexpr auto baseAmmount =
                ot::blockchain::Amount{100000000};
            auto meta = std::vector<OutpointMetadata>{};
            meta.reserve(count);
            auto output = miner_.Factory().BitcoinGenerationTransaction(
                test_chain_,
                height,
                [&] {
                    auto output = std::vector<OutputBuilder>{};
                    const auto reason =
                        client_1_.Factory().PasswordPrompt(__func__);
                    const auto keys = std::set<ot::blockchain::crypto::Key>{};

                    for (auto i = Index{0}; i < Index{count}; ++i) {
                        const auto index = account_.Reserve(
                            Subchain::External,
                            client_1_.Factory().PasswordPrompt(""));
                        const auto& element = account_.BalanceElement(
                            Subchain::External, index.value_or(0));
                        const auto key = element.Key();

                        OT_ASSERT(key);

                        switch (i) {
                            case 0: {
                                const auto& [bytes, value, pattern] =
                                    meta.emplace_back(
                                        client_1_.Factory().Data(
                                            element.Key()->PublicKey()),
                                        baseAmmount + i,
                                        Pattern::PayToPubkey);
                                output.emplace_back(
                                    value,
                                    miner_.Factory().BitcoinScriptP2PK(
                                        test_chain_, *key),
                                    keys);
                            } break;
                            default: {
                                const auto& [bytes, value, pattern] =
                                    meta.emplace_back(
                                        element.PubkeyHash(),
                                        baseAmmount + i,
                                        Pattern::PayToPubkeyHash);
                                output.emplace_back(
                                    value,
                                    miner_.Factory().BitcoinScriptP2PKH(
                                        test_chain_, *key),
                                    keys);
                            }
                        }
                    }

                    return output;
                }(),
                coinbase_fun_);

            OT_ASSERT(output);

            const auto& txid = transactions_.emplace_back(output->ID()).get();

            for (auto i = Index{0}; i < Index{count}; ++i) {
                auto& [bytes, amount, pattern] = meta.at(i);
                expected_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(txid.Bytes(), i),
                    std::forward_as_tuple(
                        std::move(bytes),
                        std::move(amount),
                        std::move(pattern)));
            }

            return output;
        })
        , listener_([&]() -> ScanListener& {
            if (!listener_p_) {
                listener_p_ = std::make_unique<ScanListener>(client_1_);
            }

            OT_ASSERT(listener_p_);

            return *listener_p_;
        }())
    {
    }
};

ot::Nym_p Regtest_fixture_hd::alex_p_{};
std::deque<ot::blockchain::block::pTxid> Regtest_fixture_hd::transactions_{};
std::unique_ptr<ScanListener> Regtest_fixture_hd::listener_p_{};
Regtest_fixture_hd::Expected Regtest_fixture_hd::expected_{};

TEST_F(Regtest_fixture_hd, init_opentxs) {}

TEST_F(Regtest_fixture_hd, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_hd, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_hd, init_account_list)
{
    account_list_.expected_ += 1;
    client_1_.UI().AccountList(
        alex_.ID(), make_cb(account_list_, u8"account_list_"));
    wait_for_counter(account_list_);
    const auto& widget = client_1_.UI().AccountList(alex_.ID());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), expected_account_.str());
    EXPECT_EQ(row->Balance(), 0);
    EXPECT_EQ(row->ContractID(), expected_unit_.str());
    EXPECT_EQ(row->DisplayBalance(), u8"0 units");
    EXPECT_EQ(row->DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(row->Name(), expected_account_name_);
    EXPECT_EQ(row->NotaryID(), expected_notary_.str());
    EXPECT_EQ(row->NotaryName(), expected_notary_name_);
    EXPECT_EQ(row->Type(), expected_account_type_);
    EXPECT_EQ(row->Unit(), expected_unit_type_);
    EXPECT_TRUE(row->Last());
}

TEST_F(Regtest_fixture_hd, init_account_activity)
{
    account_activity_.expected_ += 0;
    client_1_.UI().AccountActivity(
        alex_.ID(),
        expected_account_,
        make_cb(account_activity_, u8"account_activity_"));
    wait_for_counter(account_activity_);
    const auto& widget =
        client_1_.UI().AccountActivity(alex_.ID(), expected_account_);

    EXPECT_EQ(widget.AccountID(), expected_account_.str());
    EXPECT_EQ(widget.Balance(), 0);
    EXPECT_EQ(widget.BalancePolarity(), 0);
    EXPECT_EQ(widget.ContractID(), expected_unit_.str());
    EXPECT_FALSE(widget.DepositAddress().empty());
    EXPECT_FALSE(widget.DepositAddress(test_chain_).empty());
    EXPECT_TRUE(widget.DepositAddress(ot::blockchain::Type::Bitcoin).empty());

    const auto deposit = widget.DepositChains();

    ASSERT_EQ(deposit.size(), 1);
    EXPECT_EQ(deposit.at(0), test_chain_);
    EXPECT_EQ(widget.DisplayBalance(), u8"0 units");
    EXPECT_EQ(widget.DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(widget.Name(), expected_account_name_);
    EXPECT_EQ(widget.NotaryID(), expected_notary_.str());
    EXPECT_EQ(widget.NotaryName(), expected_notary_name_);
    EXPECT_EQ(widget.SyncPercentage(), 0);

    constexpr auto progress = std::pair<int, int>{0, 0};

    EXPECT_EQ(widget.SyncProgress(), progress);
    EXPECT_EQ(widget.Type(), expected_account_type_);
    EXPECT_EQ(widget.Unit(), expected_unit_type_);
    EXPECT_TRUE(widget.ValidateAddress("mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn"));
    EXPECT_TRUE(widget.ValidateAddress("2MzQwSSnBHWHqSAqtTVQ6v47XtaisrJa1Vc"));
    EXPECT_TRUE(widget.ValidateAddress("2N2PJEucf6QY2kNFuJ4chQEBoyZWszRQE16"));
    EXPECT_FALSE(widget.ValidateAddress("17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem"));
    EXPECT_FALSE(widget.ValidateAddress("3EktnHQD7RiAE6uzMj2ZifT9YgRrkSgzQX"));
    EXPECT_FALSE(widget.ValidateAddress("LM2WMpR1Rp6j3Sa59cMXMs1SPzj9eXpGc1"));
    EXPECT_FALSE(widget.ValidateAddress("MTf4tP1TCNBn8dNkyxeBVoPrFCcVzxJvvh"));
    EXPECT_FALSE(widget.ValidateAddress("QVk4MvUu7Wb7tZ1wvAeiUvdF7wxhvpyLLK"));
    EXPECT_FALSE(widget.ValidateAddress("pS8EA1pKEVBvv3kGsSGH37R8YViBmuRCPn"));

    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Regtest_fixture_hd, mine)
{
    auto future1 = listener_.get_future(account_, Subchain::External, 1);
    auto future2 = listener_.get_future(account_, Subchain::Internal, 1);
    constexpr auto count{1};
    account_list_.expected_ += count;
    account_activity_.expected_ += (2 * count);

    EXPECT_TRUE(Mine(0, count, hd_generator_));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
}

TEST_F(Regtest_fixture_hd, first_block)
{
    const auto& blockchain =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto blockHash = blockchain.HeaderOracle().BestHash(1);

    ASSERT_FALSE(blockHash->empty());

    const auto pBlock = blockchain.BlockOracle().LoadBitcoin(blockHash).get();

    ASSERT_TRUE(pBlock);

    const auto& block = *pBlock;

    ASSERT_EQ(block.size(), 1);

    const auto pTx = block.at(0);

    ASSERT_TRUE(pTx);

    const auto& tx = *pTx;

    EXPECT_EQ(tx.ID(), transactions_.at(0));
    EXPECT_EQ(tx.BlockPosition(), 0);
    EXPECT_EQ(tx.Outputs().size(), 100);
}

TEST_F(Regtest_fixture_hd, wallet_after_receive)
{
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto& wallet = network.Wallet();
    const auto& nym = alex_.ID();
    const auto& account = account_.ID();
    const auto blankNym = client_1_.Factory().NymID();
    const auto blankAccount = client_1_.Factory().Identifier();
    using Balance = ot::blockchain::Balance;
    const auto balance = Balance{10000004950, 10000004950};
    const auto noBalance = Balance{0, 0};

    EXPECT_EQ(wallet.GetBalance(), balance);
    EXPECT_EQ(network.GetBalance(), balance);
    EXPECT_EQ(wallet.GetBalance(nym), balance);
    EXPECT_EQ(network.GetBalance(nym), balance);
    EXPECT_EQ(wallet.GetBalance(nym, account), balance);
    EXPECT_EQ(wallet.GetBalance(blankNym), noBalance);
    EXPECT_EQ(network.GetBalance(blankNym), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(nym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, account), noBalance);

    using TxoState = ot::blockchain::node::Wallet::TxoState;
    auto type = TxoState::All;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 100u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 100u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 100u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::UnconfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::UnconfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::ConfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 100u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 100u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 100u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::ConfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);
}

TEST_F(Regtest_fixture_hd, account_list_after_receive)
{
    wait_for_counter(account_list_);
    const auto& widget = client_1_.UI().AccountList(alex_.ID());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), expected_account_.str());
    EXPECT_EQ(row->Balance(), 10000004950);
    EXPECT_EQ(row->ContractID(), expected_unit_.str());
    EXPECT_EQ(row->DisplayBalance(), u8"100.000\u202F049\u202F5 units");
    EXPECT_EQ(row->DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(row->Name(), expected_account_name_);
    EXPECT_EQ(row->NotaryID(), expected_notary_.str());
    EXPECT_EQ(row->NotaryName(), expected_notary_name_);
    EXPECT_EQ(row->Type(), expected_account_type_);
    EXPECT_EQ(row->Unit(), expected_unit_type_);
    EXPECT_TRUE(row->Last());
}

TEST_F(Regtest_fixture_hd, account_activity_after_receive)
{
    wait_for_counter(account_activity_);
    const auto& widget =
        client_1_.UI().AccountActivity(alex_.ID(), expected_account_);

    EXPECT_EQ(widget.AccountID(), expected_account_.str());
    EXPECT_EQ(widget.Balance(), 10000004950);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(widget.ContractID(), expected_unit_.str());
    EXPECT_FALSE(widget.DepositAddress().empty());
    EXPECT_FALSE(widget.DepositAddress(test_chain_).empty());
    EXPECT_TRUE(widget.DepositAddress(ot::blockchain::Type::Bitcoin).empty());

    const auto deposit = widget.DepositChains();

    ASSERT_EQ(deposit.size(), 1);
    EXPECT_EQ(deposit.at(0), test_chain_);
    EXPECT_EQ(widget.DisplayBalance(), u8"100.000\u202F049\u202F5 units");
    EXPECT_EQ(widget.DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(widget.Name(), expected_account_name_);
    EXPECT_EQ(widget.NotaryID(), expected_notary_.str());
    EXPECT_EQ(widget.NotaryName(), expected_notary_name_);
    EXPECT_EQ(widget.SyncPercentage(), 100);

    constexpr auto progress = std::pair<int, int>{1, 1};

    EXPECT_EQ(widget.SyncProgress(), progress);
    EXPECT_EQ(widget.Type(), expected_account_type_);
    EXPECT_EQ(widget.Unit(), expected_unit_type_);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 10000004950);
    EXPECT_EQ(row->Contacts().size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"100.000\u202F049\u202F5 units");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Unit Test Simulation transaction");
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_EQ(row->UUID(), ot::blockchain::HashToNumber(transactions_.at(0)));
    EXPECT_TRUE(row->Last());
}

#if OT_WITH_RPC
TEST_F(Regtest_fixture_hd, rpc_account_list)
{
    const auto index{client_1_.Instance()};
    const auto command = ot::rpc::request::ListAccounts{index};
    const auto base = ot_.RPC(command);
    const auto& response = base->asListAccounts();
    const auto& codes = response.ResponseCodes();
    const auto expected = [&] {
        auto out = std::set<std::string>{};
        const auto& id =
            client_1_.Blockchain().Account(alex_.ID(), test_chain_).AccountID();
        out.emplace(id.str());

        return out;
    }();
    const auto& ids = response.AccountIDs();

    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::success);
    EXPECT_EQ(ids.size(), 1);

    for (const auto& id : ids) { EXPECT_EQ(expected.count(id), 1); }

    {
        auto bytes = ot::Space{};

        EXPECT_TRUE(command.Serialize(ot::writer(bytes)));
        EXPECT_TRUE(base->Serialize(ot::writer(bytes)));

        auto recovered = ot::rpc::response::Factory(ot::reader(bytes));

        EXPECT_EQ(recovered->Type(), ot::rpc::CommandType::list_accounts);
    }
}

TEST_F(Regtest_fixture_hd, rpc_account_activity_receive)
{
    const auto index{client_1_.Instance()};
    const auto command =
        ot::rpc::request::GetAccountActivity{index, {expected_account_.str()}};
    const auto base = ot_.RPC(command);
    const auto& response = base->asGetAccountActivity();
    const auto& codes = response.ResponseCodes();
    const auto& events = response.Activity();

    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::success);
    ASSERT_EQ(events.size(), 1);

    {
        const auto& event = events.at(0);

        EXPECT_EQ(event.AccountID(), expected_account_.str());
        EXPECT_EQ(event.ConfirmedAmount(), 10000004950);
        EXPECT_EQ(event.ContactID(), "");
        EXPECT_EQ(event.Memo(), "");
        EXPECT_EQ(event.PendingAmount(), 10000004950);
        EXPECT_EQ(event.State(), 0);
        EXPECT_EQ(event.Type(), ot::rpc::AccountEventType::incoming_blockchain);
        EXPECT_EQ(
            event.UUID(), ot::blockchain::HashToNumber(transactions_.at(0)));
        EXPECT_EQ(event.WorkflowID(), "");
    }
    {
        auto bytes = ot::Space{};

        EXPECT_TRUE(base->Serialize(ot::writer(bytes)));

        auto recovered = ot::rpc::response::Factory(ot::reader(bytes));

        EXPECT_EQ(
            recovered->Type(), ot::rpc::CommandType::get_account_activity);

        const auto& rEvents = recovered->asGetAccountActivity().Activity();

        ASSERT_EQ(rEvents.size(), 1);

        {
            const auto& event = rEvents.at(0);

            EXPECT_EQ(event.AccountID(), expected_account_.str());
            EXPECT_EQ(event.ConfirmedAmount(), 10000004950);
            EXPECT_EQ(event.ContactID(), "");
            EXPECT_EQ(event.Memo(), "");
            EXPECT_EQ(event.PendingAmount(), 10000004950);
            EXPECT_EQ(event.State(), 0);
            EXPECT_EQ(
                event.Type(), ot::rpc::AccountEventType::incoming_blockchain);
            EXPECT_EQ(
                event.UUID(),
                ot::blockchain::HashToNumber(transactions_.at(0)));
            EXPECT_EQ(event.WorkflowID(), "");
        }
    }
}

// TODO more rpc tests go here
#endif  // OT_WITH_RPC

TEST_F(Regtest_fixture_hd, spend)
{
    account_list_.expected_ += 1;
    account_activity_.expected_ += 2;
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto& widget =
        client_1_.UI().AccountActivity(alex_.ID(), expected_account_);
    constexpr auto sendAmount{u8"14 units"};
    constexpr auto address{"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn"};

    ASSERT_FALSE(widget.ValidateAmount(sendAmount).empty());
    ASSERT_TRUE(widget.ValidateAddress(address));

    auto future =
        network.SendToAddress(alex_.ID(), address, 1400000000, memo_outgoing_);
    const auto& txid = transactions_.emplace_back(future.get().second);

    EXPECT_FALSE(txid->empty());

    const auto& element = account_.BalanceElement(Subchain::Internal, 0);
    constexpr auto amount = ot::blockchain::Amount{99997807};
    expected_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(txid->Bytes(), 0),
        std::forward_as_tuple(
            element.PubkeyHash(), amount, Pattern::PayToPubkeyHash));
}

TEST_F(Regtest_fixture_hd, wallet_after_unconfirmed_spend)
{
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto& wallet = network.Wallet();
    const auto& nym = alex_.ID();
    const auto& account = account_.ID();
    const auto blankNym = client_1_.Factory().NymID();
    const auto blankAccount = client_1_.Factory().Identifier();
    using Balance = ot::blockchain::Balance;
    const auto balance = Balance{10000004950, 8600002652};
    const auto noBalance = Balance{0, 0};

    EXPECT_EQ(wallet.GetBalance(), balance);
    EXPECT_EQ(network.GetBalance(), balance);
    EXPECT_EQ(wallet.GetBalance(nym), balance);
    EXPECT_EQ(network.GetBalance(nym), balance);
    EXPECT_EQ(wallet.GetBalance(nym, account), balance);
    EXPECT_EQ(wallet.GetBalance(blankNym), noBalance);
    EXPECT_EQ(network.GetBalance(blankNym), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(nym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, account), noBalance);

    using TxoState = ot::blockchain::node::Wallet::TxoState;
    auto type = TxoState::All;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 101u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 101u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 101u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::UnconfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 1u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 1u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 1u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::UnconfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 15u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 15u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 15u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::ConfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 85u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 85u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 85u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::ConfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);
}

TEST_F(Regtest_fixture_hd, account_list_after_spend)
{
    wait_for_counter(account_list_);
    const auto& widget = client_1_.UI().AccountList(alex_.ID());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), expected_account_.str());
    EXPECT_EQ(row->Balance(), 8600002652);
    EXPECT_EQ(row->ContractID(), expected_unit_.str());
    EXPECT_EQ(row->DisplayBalance(), u8"86.000\u202F026\u202F52 units");
    EXPECT_EQ(row->DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(row->Name(), expected_account_name_);
    EXPECT_EQ(row->NotaryID(), expected_notary_.str());
    EXPECT_EQ(row->NotaryName(), expected_notary_name_);
    EXPECT_EQ(row->Type(), expected_account_type_);
    EXPECT_EQ(row->Unit(), expected_unit_type_);
    EXPECT_TRUE(row->Last());
}

TEST_F(Regtest_fixture_hd, account_activity_after_unconfirmed_spend)
{
    wait_for_counter(account_activity_);
    const auto& widget =
        client_1_.UI().AccountActivity(alex_.ID(), expected_account_);

    EXPECT_EQ(widget.AccountID(), expected_account_.str());
    EXPECT_EQ(widget.Balance(), 8600002652);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(widget.ContractID(), expected_unit_.str());
    EXPECT_FALSE(widget.DepositAddress().empty());
    EXPECT_FALSE(widget.DepositAddress(test_chain_).empty());
    EXPECT_TRUE(widget.DepositAddress(ot::blockchain::Type::Bitcoin).empty());

    const auto deposit = widget.DepositChains();

    ASSERT_EQ(deposit.size(), 1);
    EXPECT_EQ(deposit.at(0), test_chain_);
    EXPECT_EQ(widget.DisplayBalance(), u8"86.000\u202F026\u202F52 units");
    EXPECT_EQ(widget.DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(widget.Name(), expected_account_name_);
    EXPECT_EQ(widget.NotaryID(), expected_notary_.str());
    EXPECT_EQ(widget.NotaryName(), expected_notary_name_);
    EXPECT_EQ(widget.SyncPercentage(), 100);

    constexpr auto progress = std::pair<int, int>{1, 1};

    EXPECT_EQ(widget.SyncProgress(), progress);
    EXPECT_EQ(widget.Type(), expected_account_type_);
    EXPECT_EQ(widget.Unit(), expected_unit_type_);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), -1400002298);
    EXPECT_EQ(row->Contacts().size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"-14.000\u202F022\u202F98 units");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Outgoing Unit Test Simulation transaction");
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    transactions_.emplace_back(
        ot::blockchain::NumberToHash(client_1_, row->UUID()));
    row = widget.Next();

    EXPECT_EQ(row->Amount(), 10000004950);
    EXPECT_EQ(row->Contacts().size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"100.000\u202F049\u202F5 units");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Unit Test Simulation transaction");
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_EQ(row->UUID(), ot::blockchain::HashToNumber(transactions_.at(0)));
}

TEST_F(Regtest_fixture_hd, confirm)
{
    auto future1 = listener_.get_future(account_, Subchain::External, 2);
    auto future2 = listener_.get_future(account_, Subchain::Internal, 2);
    account_list_.expected_ += 0;
    account_activity_.expected_ += 1;
    const auto& txid = transactions_.at(1).get();
    const auto extra = [&] {
        auto output = std::vector<Transaction>{};
        const auto& pTX = output.emplace_back(
            client_1_.Blockchain().LoadTransactionBitcoin(txid));

        OT_ASSERT(pTX);

        return output;
    }();

    EXPECT_TRUE(Mine(1, 1, default_, extra));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
}

TEST_F(Regtest_fixture_hd, wallet_after_confirmed_spend)
{
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto& wallet = network.Wallet();
    const auto& nym = alex_.ID();
    const auto& account = account_.ID();
    const auto blankNym = client_1_.Factory().NymID();
    const auto blankAccount = client_1_.Factory().Identifier();
    using Balance = ot::blockchain::Balance;
    const auto balance = Balance{8600002652, 8600002652};
    const auto noBalance = Balance{0, 0};

    EXPECT_EQ(wallet.GetBalance(), balance);
    EXPECT_EQ(network.GetBalance(), balance);
    EXPECT_EQ(wallet.GetBalance(nym), balance);
    EXPECT_EQ(network.GetBalance(nym), balance);
    EXPECT_EQ(wallet.GetBalance(nym, account), balance);
    EXPECT_EQ(wallet.GetBalance(blankNym), noBalance);
    EXPECT_EQ(network.GetBalance(blankNym), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(nym, blankAccount), noBalance);
    EXPECT_EQ(wallet.GetBalance(blankNym, account), noBalance);

    using TxoState = ot::blockchain::node::Wallet::TxoState;
    auto type = TxoState::All;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 101u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 101u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 101u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::UnconfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::UnconfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::ConfirmedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 86u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 86u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 86u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::ConfirmedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 15u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 15u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 15u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, type)));
    EXPECT_TRUE(TestUTXOs(expected_, wallet.GetOutputs(nym, account, type)));

    type = TxoState::OrphanedNew;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);

    type = TxoState::OrphanedSpend;

    EXPECT_EQ(wallet.GetOutputs(type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, account, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(nym, blankAccount, type).size(), 0u);
    EXPECT_EQ(wallet.GetOutputs(blankNym, account, type).size(), 0u);
}

TEST_F(Regtest_fixture_hd, account_activity_after_confirmed_spend)
{
    wait_for_counter(account_activity_);
    const auto& widget =
        client_1_.UI().AccountActivity(alex_.ID(), expected_account_);

    EXPECT_EQ(widget.AccountID(), expected_account_.str());
    EXPECT_EQ(widget.Balance(), 8600002652);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(widget.ContractID(), expected_unit_.str());
    EXPECT_FALSE(widget.DepositAddress().empty());
    EXPECT_FALSE(widget.DepositAddress(test_chain_).empty());
    EXPECT_TRUE(widget.DepositAddress(ot::blockchain::Type::Bitcoin).empty());

    const auto deposit = widget.DepositChains();

    ASSERT_EQ(deposit.size(), 1);
    EXPECT_EQ(deposit.at(0), test_chain_);
    EXPECT_EQ(widget.DisplayBalance(), u8"86.000\u202F026\u202F52 units");
    EXPECT_EQ(widget.DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(widget.Name(), expected_account_name_);
    EXPECT_EQ(widget.NotaryID(), expected_notary_.str());
    EXPECT_EQ(widget.NotaryName(), expected_notary_name_);
    EXPECT_EQ(widget.SyncPercentage(), 100);

    constexpr auto progress = std::pair<int, int>{2, 2};

    EXPECT_EQ(widget.SyncProgress(), progress);
    EXPECT_EQ(widget.Type(), expected_account_type_);
    EXPECT_EQ(widget.Unit(), expected_unit_type_);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), -1400002298);
    EXPECT_EQ(row->Contacts().size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"-14.000\u202F022\u202F98 units");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Outgoing Unit Test Simulation transaction");
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    transactions_.emplace_back(
        ot::blockchain::NumberToHash(client_1_, row->UUID()));
    row = widget.Next();

    EXPECT_EQ(row->Amount(), 10000004950);
    EXPECT_EQ(row->Contacts().size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"100.000\u202F049\u202F5 units");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Unit Test Simulation transaction");
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_EQ(row->UUID(), ot::blockchain::HashToNumber(transactions_.at(0)));
}

#if OT_WITH_RPC
TEST_F(Regtest_fixture_hd, rpc_account_balance)
{
    const auto index{client_1_.Instance()};
    const auto command =
        ot::rpc::request::GetAccountBalance{index, {expected_account_.str()}};
    const auto base = ot_.RPC(command);
    const auto& response = base->asGetAccountBalance();
    const auto& codes = response.ResponseCodes();
    const auto& data = response.Balances();

    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::success);
    ASSERT_EQ(data.size(), 1);

    {
        const auto& balance = data.at(0);

        EXPECT_EQ(balance.ConfirmedBalance(), 8600002652);
        EXPECT_EQ(balance.ID(), expected_account_.str());
        EXPECT_EQ(
            balance.Issuer(),
            client_1_.UI().BlockchainIssuerID(test_chain_).str());
        EXPECT_EQ(balance.Name(), "This device");
        EXPECT_EQ(balance.Owner(), alex_.ID().str());
        EXPECT_EQ(balance.PendingBalance(), 8600002652);
        EXPECT_EQ(balance.Type(), ot::rpc::AccountType::blockchain);
        EXPECT_EQ(
            balance.Unit(), client_1_.UI().BlockchainUnitID(test_chain_).str());
    }
}

TEST_F(Regtest_fixture_hd, rpc_account_activity_spend)
{
    const auto index{client_1_.Instance()};
    const auto command =
        ot::rpc::request::GetAccountActivity{index, {expected_account_.str()}};
    const auto base = ot_.RPC(command);
    const auto& response = base->asGetAccountActivity();
    const auto& codes = response.ResponseCodes();
    const auto& events = response.Activity();

    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(codes.at(0).first, 0);
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::success);
    ASSERT_EQ(events.size(), 2);

    {
        const auto& event = events.at(0);

        EXPECT_EQ(event.AccountID(), expected_account_.str());
        EXPECT_EQ(event.ConfirmedAmount(), -1400002298);
        EXPECT_EQ(event.ContactID(), "");
        EXPECT_EQ(event.Memo(), "");
        EXPECT_EQ(event.PendingAmount(), -1400002298);
        EXPECT_EQ(event.State(), 0);
        EXPECT_EQ(event.Type(), ot::rpc::AccountEventType::outgoing_blockchain);
        EXPECT_EQ(
            event.UUID(), ot::blockchain::HashToNumber(transactions_.at(1)));
        EXPECT_EQ(event.WorkflowID(), "");
    }
    {
        const auto& event = events.at(1);

        EXPECT_EQ(event.AccountID(), expected_account_.str());
        EXPECT_EQ(event.ConfirmedAmount(), 10000004950);
        EXPECT_EQ(event.ContactID(), "");
        EXPECT_EQ(event.Memo(), "");
        EXPECT_EQ(event.PendingAmount(), 10000004950);
        EXPECT_EQ(event.State(), 0);
        EXPECT_EQ(event.Type(), ot::rpc::AccountEventType::incoming_blockchain);
        EXPECT_EQ(
            event.UUID(), ot::blockchain::HashToNumber(transactions_.at(0)));
        EXPECT_EQ(event.WorkflowID(), "");
    }
    {
        auto bytes = ot::Space{};

        EXPECT_TRUE(base->Serialize(ot::writer(bytes)));

        auto recovered = ot::rpc::response::Factory(ot::reader(bytes));

        EXPECT_EQ(
            recovered->Type(), ot::rpc::CommandType::get_account_activity);

        const auto& rEvents = recovered->asGetAccountActivity().Activity();

        ASSERT_EQ(rEvents.size(), 2);

        {
            const auto& event = rEvents.at(0);

            EXPECT_EQ(event.AccountID(), expected_account_.str());
            EXPECT_EQ(event.ConfirmedAmount(), -1400002298);
            EXPECT_EQ(event.ContactID(), "");
            EXPECT_EQ(event.Memo(), "");
            EXPECT_EQ(event.PendingAmount(), -1400002298);
            EXPECT_EQ(event.State(), 0);
            EXPECT_EQ(
                event.Type(), ot::rpc::AccountEventType::outgoing_blockchain);
            EXPECT_EQ(
                event.UUID(),
                ot::blockchain::HashToNumber(transactions_.at(1)));
            EXPECT_EQ(event.WorkflowID(), "");
        }
        {
            const auto& event = rEvents.at(1);

            EXPECT_EQ(event.AccountID(), expected_account_.str());
            EXPECT_EQ(event.ConfirmedAmount(), 10000004950);
            EXPECT_EQ(event.ContactID(), "");
            EXPECT_EQ(event.Memo(), "");
            EXPECT_EQ(event.PendingAmount(), 10000004950);
            EXPECT_EQ(event.State(), 0);
            EXPECT_EQ(
                event.Type(), ot::rpc::AccountEventType::incoming_blockchain);
            EXPECT_EQ(
                event.UUID(),
                ot::blockchain::HashToNumber(transactions_.at(0)));
            EXPECT_EQ(event.WorkflowID(), "");
        }
    }
}
#endif  // OT_WITH_RPC

TEST_F(Regtest_fixture_hd, account_list_after_confirmed_spend)
{
    wait_for_counter(account_list_);
    const auto& widget = client_1_.UI().AccountList(alex_.ID());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), expected_account_.str());
    EXPECT_EQ(row->Balance(), 8600002652);
    EXPECT_EQ(row->ContractID(), expected_unit_.str());
    EXPECT_EQ(row->DisplayBalance(), u8"86.000\u202F026\u202F52 units");
    EXPECT_EQ(row->DisplayUnit(), expected_display_unit_);
    EXPECT_EQ(row->Name(), expected_account_name_);
    EXPECT_EQ(row->NotaryID(), expected_notary_.str());
    EXPECT_EQ(row->NotaryName(), expected_notary_name_);
    EXPECT_EQ(row->Type(), expected_account_type_);
    EXPECT_EQ(row->Unit(), expected_unit_type_);
    EXPECT_TRUE(row->Last());
}

TEST_F(Regtest_fixture_hd, shutdown) { Shutdown(); }
}  // namespace ottest
