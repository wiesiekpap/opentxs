// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <deque>

#include "UIHelpers.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/client/blockchain/BalanceTree.hpp"
#include "opentxs/api/client/blockchain/HD.hpp"
#include "opentxs/api/client/blockchain/Subchain.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/client/BlockOracle.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"

Counter account_activity_{};

class Regtest_fixture_hd : public Regtest_fixture_normal
{
protected:
    using Subchain = ot::api::client::blockchain::Subchain;

    static ot::Nym_p alex_p_;
    static std::deque<ot::blockchain::block::pTxid> transactions_;

    const ot::identity::Nym& alex_;
    const Generator hd_generator_;

    auto Shutdown() noexcept -> void final
    {
        transactions_.clear();
        alex_p_.reset();
        Regtest_fixture_normal::Shutdown();
    }

    Regtest_fixture_hd()
        : alex_([&]() -> const ot::identity::Nym& {
            if (!alex_p_) {
                const auto reason =
                    client_.Factory().PasswordPrompt(__FUNCTION__);

                alex_p_ = client_.Wallet().Nym(reason, "Alex");

                OT_ASSERT(alex_p_)

                client_.Blockchain().NewHDSubaccount(
                    alex_p_->ID(),
                    ot::BlockchainAccountType::BIP44,
                    test_chain_,
                    reason);
            }

            OT_ASSERT(alex_p_)

            return *alex_p_;
        }())
        , hd_generator_([&](Height height) -> Transaction {
            using OutputBuilder = ot::api::Factory::OutputBuilder;

            auto output = miner_.Factory().BitcoinGenerationTransaction(
                test_chain_,
                height,
                [&] {
                    auto output = std::vector<OutputBuilder>{};
                    const auto& account = client_.Blockchain()
                                              .Account(alex_.ID(), test_chain_)
                                              .GetHD()
                                              .at(0);
                    const auto reason =
                        client_.Factory().PasswordPrompt(__FUNCTION__);
                    const auto keys =
                        std::set<ot::api::client::blockchain::Key>{};
                    static const auto amount =
                        ot::blockchain::Amount{100000000};
                    using Index = ot::Bip32Index;

                    for (auto i = Index{0}; i < Index{100}; ++i) {
                        const auto& element =
                            account.BalanceElement(Subchain::External, i);
                        const auto key = element.Key();

                        OT_ASSERT(key);

                        switch (i) {
                            case 0: {
                                output.emplace_back(
                                    amount + i,
                                    miner_.Factory().BitcoinScriptP2PK(
                                        test_chain_, *key),
                                    keys);
                            } break;
                            default: {
                                output.emplace_back(
                                    amount + i,
                                    miner_.Factory().BitcoinScriptP2PKH(
                                        test_chain_, *key),
                                    keys);
                            }
                        }
                    }

                    return output;
                }(),
                "The Industrial Revolution and its consequences have been a "
                "disaster for the human race.");

            OT_ASSERT(output);

            transactions_.emplace_back(output->ID());

            return output;
        })
    {
        auto target = ot::Bip32Index{99};
        const auto& account =
            client_.Blockchain().Account(alex_.ID(), test_chain_).GetHD().at(0);
        const auto reason = client_.Factory().PasswordPrompt(__FUNCTION__);
        auto index = account.LastGenerated(Subchain::External);

        while ((false == index.has_value()) || (index.value() < target)) {
            index = account.GenerateNext(Subchain::External, reason);

            OT_ASSERT(index.has_value());
        }
    }
};

ot::Nym_p Regtest_fixture_hd::alex_p_{};
std::deque<ot::blockchain::block::pTxid> Regtest_fixture_hd::transactions_{};

namespace
{
TEST_F(Regtest_fixture_hd, init_opentxs) {}

TEST_F(Regtest_fixture_hd, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_hd, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_hd, init_widgets)
{
    account_activity_.expected_ += 0;
    client_.UI().AccountActivity(
        alex_.ID(),
        client_.UI().BlockchainAccountID(test_chain_),
        make_cb(account_activity_, u8"account_activity_"));
}

TEST_F(Regtest_fixture_hd, mine)
{
    constexpr auto count{1};
    account_activity_.expected_ += (2 * count);

    EXPECT_TRUE(Mine(0, count, hd_generator_));
}

TEST_F(Regtest_fixture_hd, block)
{
    const auto& blockchain = client_.Blockchain().GetChain(test_chain_);
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
    ASSERT_EQ(tx.Outputs().size(), 100);
}

TEST_F(Regtest_fixture_hd, account_activity)
{
    wait_for_counter(account_activity_);

    const auto& accountID = client_.UI().BlockchainAccountID(test_chain_);
    const auto& widget = client_.UI().AccountActivity(alex_.ID(), accountID);

    EXPECT_EQ(widget.AccountID(), accountID.str());
    EXPECT_EQ(widget.Balance(), 10000004950);
    EXPECT_EQ(widget.BalancePolarity(), 1);
    EXPECT_EQ(
        widget.ContractID(), client_.UI().BlockchainUnitID(test_chain_).str());
    EXPECT_FALSE(widget.DepositAddress().empty());
    EXPECT_FALSE(widget.DepositAddress(test_chain_).empty());
    EXPECT_TRUE(widget.DepositAddress(ot::blockchain::Type::Bitcoin).empty());

    const auto deposit = widget.DepositChains();

    ASSERT_EQ(deposit.size(), 1);
    EXPECT_EQ(deposit.at(0), test_chain_);
    EXPECT_EQ(widget.DisplayBalance(), u8"100.000 049 5 units");
    EXPECT_EQ(widget.DisplayUnit(), "UNITTEST");
    EXPECT_EQ(widget.Name(), "This device");
    EXPECT_EQ(
        widget.NotaryID(), client_.UI().BlockchainNotaryID(test_chain_).str());
    EXPECT_EQ(widget.NotaryName(), "Unit Test Simulation");
    EXPECT_EQ(widget.SyncPercentage(), 100);

    constexpr auto progress = std::pair<int, int>{1, 1};

    EXPECT_EQ(widget.SyncProgress(), progress);
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::proto::CITEMTYPE_REGTEST);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 10000004950);
    EXPECT_EQ(row->Contacts().size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"100.000 049 5 units");
    EXPECT_EQ(row->Memo(), "");
    EXPECT_EQ(row->Workflow(), "");
    EXPECT_EQ(row->Text(), "Incoming Unit Test Simulation transaction");
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_EQ(row->UUID(), transactions_.at(0)->asHex());
    EXPECT_TRUE(row->Last());
}

TEST_F(Regtest_fixture_hd, shutdown) { Shutdown(); }
}  // namespace
