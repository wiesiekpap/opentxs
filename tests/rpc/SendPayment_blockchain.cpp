// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <array>

#include "blockchain/regtest/Helpers.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/api/client/Blockchain.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/server/Manager.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/HDProtocol.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/rpc/CommandType.hpp"
#include "opentxs/rpc/PaymentType.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/SendPayment.hpp"
#include "opentxs/rpc/response/SendPayment.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
class RPC_BC : public Regtest_fixture_normal, public RPC_fixture
{
protected:
    using Subchain = ot::blockchain::crypto::Subchain;

    static ot::Nym_p alex_p_;
    static std::deque<ot::blockchain::block::pTxid> transactions_;
    static std::unique_ptr<ScanListener> listener_p_;

    const ot::identity::Nym& alex_;
    const ot::blockchain::crypto::HD& account_;
    const Generator mine_to_alex_;
    ScanListener& listener_;

    auto Cleanup() noexcept -> void final
    {
        listener_p_.reset();
        transactions_.clear();
        alex_p_.reset();
        Regtest_fixture_normal::Shutdown();
        RPC_fixture::Cleanup();
    }

    RPC_BC()
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
        , mine_to_alex_([&](Height height) -> Transaction {
            using OutputBuilder = ot::api::Factory::OutputBuilder;
            static constexpr auto baseAmount =
                ot::blockchain::Amount{10000000000};
            auto output = miner_.Factory().BitcoinGenerationTransaction(
                test_chain_,
                height,
                [&] {
                    auto output = std::vector<OutputBuilder>{};
                    const auto reason =
                        client_1_.Factory().PasswordPrompt(__func__);
                    const auto keys = std::set<ot::blockchain::crypto::Key>{};
                    const auto index =
                        account_.Reserve(Subchain::External, reason);

                    EXPECT_TRUE(index.has_value());

                    const auto& element = account_.BalanceElement(
                        Subchain::External, index.value_or(0));
                    const auto key = element.Key();

                    OT_ASSERT(key);

                    output.emplace_back(
                        baseAmount,
                        miner_.Factory().BitcoinScriptP2PK(test_chain_, *key),
                        keys);

                    return output;
                }(),
                coinbase_fun_);

            OT_ASSERT(output);

            transactions_.emplace_back(output->ID()).get();

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

ot::Nym_p RPC_BC::alex_p_{};
std::deque<ot::blockchain::block::pTxid> RPC_BC::transactions_{};
std::unique_ptr<ScanListener> RPC_BC::listener_p_{};

TEST_F(RPC_BC, preconditions)
{
    ASSERT_TRUE(Start());
    ASSERT_TRUE(Connect());

    {
        auto future1 = listener_.get_future(account_, Subchain::External, 1);
        auto future2 = listener_.get_future(account_, Subchain::Internal, 1);
        constexpr auto count{1};

        EXPECT_TRUE(Mine(0, count, mine_to_alex_));
        EXPECT_TRUE(listener_.wait(future1));
        EXPECT_TRUE(listener_.wait(future2));
    }
}

TEST_F(RPC_BC, blockchain_payment)
{
    const auto index{client_1_.Instance()};
    constexpr auto address{"n4VQ5YdHf7hLQ2gWQYYrcxoE5B7nWuDFNF"};
    constexpr auto amount = ot::Amount{140000};
    const auto account = account_.Parent().AccountID().str();
    const auto command =
        ot::rpc::request::SendPayment{index, account, address, amount};
    const auto& send = command.asSendPayment();
    const auto base = RPC_fixture::ot_.RPC(command);
    const auto& response = base->asSendPayment();
    const auto& codes = response.ResponseCodes();
    const auto& pending = response.Pending();

    EXPECT_EQ(command.AssociatedNyms().size(), 0);
    EXPECT_NE(command.Cookie().size(), 0);
    EXPECT_EQ(command.Session(), index);
    EXPECT_EQ(command.Type(), ot::rpc::CommandType::send_payment);
    EXPECT_NE(command.Version(), 0);
    EXPECT_EQ(send.Amount(), amount);
    EXPECT_EQ(send.AssociatedNyms().size(), 0);
    EXPECT_EQ(send.Cookie(), command.Cookie());
    EXPECT_EQ(send.DestinationAccount(), address);
    EXPECT_EQ(send.Session(), command.Session());
    EXPECT_EQ(send.SourceAccount(), account);
    EXPECT_EQ(send.Type(), command.Type());
    EXPECT_EQ(send.Version(), command.Version());
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
    EXPECT_EQ(codes.at(0).second, ot::rpc::ResponseCode::txid);
    ASSERT_EQ(pending.size(), 1);
    ASSERT_EQ(codes.size(), 1);
    EXPECT_EQ(pending.at(0).first, 0);
    EXPECT_NE(pending.at(0).second.size(), 0);

    transactions_.emplace_back(
        client_1_.Factory().Data(pending.at(0).second, ot::StringStyle::Hex));
}

TEST_F(RPC_BC, postconditions)
{
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto [confirmed, unconfirmed] = network.GetBalance(alex_.ID());

    EXPECT_EQ(confirmed, 10000000000);
    EXPECT_EQ(unconfirmed, 9999859774);
}

TEST_F(RPC_BC, cleanup) { Cleanup(); }
}  // namespace ottest
