// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#if OT_QT
#include <QObject>
#include <QString>
#include <QVariant>
#endif  // OT_QT
#include <gtest/gtest.h>
#include <atomic>

#include "opentxs/OT.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/blockchain/Blockchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/ui/BlockchainSelectionItem.hpp"
#include "opentxs/ui/Blockchains.hpp"
#if OT_QT
#include "opentxs/ui/qt/BlockchainSelection.hpp"
#endif  // OT_QT
#include "ui/Helpers.hpp"

namespace ottest
{
Counter counter_full_{};
Counter counter_main_{};
Counter counter_test_{};

class Test_BlockchainSelector : public ::testing::Test
{
public:
    const ot::api::client::Manager& client_;
    const ot::ui::BlockchainSelection& full_;
    const ot::ui::BlockchainSelection& main_;
    const ot::ui::BlockchainSelection& test_;

    Test_BlockchainSelector()
        : client_(ot::Context().StartClient(0))
        , full_([&]() -> auto& {
            static std::atomic_bool init{true};
            static auto cb =
                make_cb(counter_full_, "Blockchain selector (full)");

            if (init) {
                counter_full_.expected_ = 7;
                init = false;
            }

            return client_.UI().BlockchainSelection(
                ot::ui::Blockchains::All, cb);
        }())
        , main_([&]() -> auto& {
            static std::atomic_bool init{true};
            static auto cb =
                make_cb(counter_main_, "Blockchain selector (main)");

            if (init) {
                counter_main_.expected_ = 4;
                init = false;
            }

            return client_.UI().BlockchainSelection(
                ot::ui::Blockchains::Main, cb);
        }())
        , test_([&]() -> auto& {
            static std::atomic_bool init{true};
            static auto cb =
                make_cb(counter_test_, "Blockchain selector (test)");

            if (init) {
                counter_test_.expected_ = 3;
                init = false;
            }

            return client_.UI().BlockchainSelection(
                ot::ui::Blockchains::Test, cb);
        }())
    {
    }
};

TEST_F(Test_BlockchainSelector, initialize_opentxs)
{
    ASSERT_TRUE(wait_for_counter(counter_full_));
    ASSERT_TRUE(wait_for_counter(counter_main_));
    ASSERT_TRUE(wait_for_counter(counter_test_));
}

TEST_F(Test_BlockchainSelector, initial_state_all)
{
    auto& model = full_;
    auto row = model.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin Cash");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::BitcoinCash);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Litecoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Litecoin);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "PKT");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::PKT);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin (testnet3)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin_testnet3);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin Cash (testnet3)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::BitcoinCash_testnet3);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Litecoin (testnet4)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Litecoin_testnet4);
    }

    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainSelector, initial_state_all_qt)
{
    const auto* pWidget =
        client_.UI().BlockchainSelectionQt(ot::ui::Blockchains::All);

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    ASSERT_EQ(widget.columnCount(), 1);
    ASSERT_EQ(widget.rowCount(), 7);

    using Model = ot::ui::BlockchainSelectionQt;

    {
        const auto row{0};
        const auto expected{ot::blockchain::Type::Bitcoin};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{1};
        const auto expected{ot::blockchain::Type::BitcoinCash};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{2};
        const auto expected{ot::blockchain::Type::Litecoin};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{3};
        const auto expected{ot::blockchain::Type::PKT};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{4};
        const auto expected{ot::blockchain::Type::Bitcoin_testnet3};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
    {
        const auto row{5};
        const auto expected{ot::blockchain::Type::BitcoinCash_testnet3};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
    {
        const auto row{6};
        const auto expected{ot::blockchain::Type::Litecoin_testnet4};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainSelector, initial_state_main)
{
    auto& model = main_;
    auto row = model.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin Cash");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::BitcoinCash);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Litecoin");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Litecoin);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "PKT");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_FALSE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::PKT);
    }

    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainSelector, initial_state_main_qt)
{
    const auto* pWidget =
        client_.UI().BlockchainSelectionQt(ot::ui::Blockchains::Main);

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    ASSERT_EQ(widget.columnCount(), 1);
    ASSERT_EQ(widget.rowCount(), 4);

    using Model = ot::ui::BlockchainSelectionQt;

    {
        const auto row{0};
        const auto expected{ot::blockchain::Type::Bitcoin};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{1};
        const auto expected{ot::blockchain::Type::BitcoinCash};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{2};
        const auto expected{ot::blockchain::Type::Litecoin};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{3};
        const auto expected{ot::blockchain::Type::PKT};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainSelector, initial_state_test)
{
    auto& model = test_;
    auto row = model.First();

    ASSERT_TRUE(row->Valid());

    {
        EXPECT_EQ(row->Name(), "Bitcoin (testnet3)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin_testnet3);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Bitcoin Cash (testnet3)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::BitcoinCash_testnet3);
    }

    ASSERT_FALSE(row->Last());

    row = model.Next();

    {
        EXPECT_EQ(row->Name(), "Litecoin (testnet4)");
        EXPECT_FALSE(row->IsEnabled());
        EXPECT_TRUE(row->IsTestnet());
        EXPECT_EQ(row->Type(), ot::blockchain::Type::Litecoin_testnet4);
    }

    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainSelector, initial_state_test_qt)
{
    const auto* pWidget =
        client_.UI().BlockchainSelectionQt(ot::ui::Blockchains::Test);

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    ASSERT_EQ(widget.columnCount(), 1);
    ASSERT_EQ(widget.rowCount(), 3);

    using Model = ot::ui::BlockchainSelectionQt;

    {
        const auto row{0};
        const auto expected{ot::blockchain::Type::Bitcoin_testnet3};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
    {
        const auto row{1};
        const auto expected{ot::blockchain::Type::BitcoinCash_testnet3};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
    {
        const auto row{2};
        const auto expected{ot::blockchain::Type::Litecoin_testnet4};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainSelector, disable_disabled)
{
    {
        auto& model = full_;

        EXPECT_TRUE(model.Disable(ot::blockchain::Type::Bitcoin));

        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_FALSE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
    {
        auto& model = main_;
        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_FALSE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
}

TEST_F(Test_BlockchainSelector, enable_disabled)
{
    counter_full_.expected_ += 1;
    counter_main_.expected_ += 1;

    {
        auto& model = full_;

        EXPECT_TRUE(model.Enable(ot::blockchain::Type::Bitcoin));
        ASSERT_TRUE(wait_for_counter(counter_full_));

        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_TRUE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
    {
        auto& model = main_;

        ASSERT_TRUE(wait_for_counter(counter_main_));

        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_TRUE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
}

TEST_F(Test_BlockchainSelector, enable_enabled)
{
    {
        auto& model = full_;

        EXPECT_TRUE(model.Enable(ot::blockchain::Type::Bitcoin));

        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_TRUE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
    {
        auto& model = main_;
        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_TRUE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
}

TEST_F(Test_BlockchainSelector, disable_enabled)
{
    counter_full_.expected_ += 1;
    counter_main_.expected_ += 1;

    {
        auto& model = full_;

        EXPECT_TRUE(model.Disable(ot::blockchain::Type::Bitcoin));
        ASSERT_TRUE(wait_for_counter(counter_full_));

        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_FALSE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
    {
        auto& model = main_;

        ASSERT_TRUE(wait_for_counter(counter_main_));

        auto row = model.First();

        ASSERT_TRUE(row->Valid());

        {
            EXPECT_EQ(row->Name(), "Bitcoin");
            EXPECT_FALSE(row->IsEnabled());
            EXPECT_FALSE(row->IsTestnet());
            EXPECT_EQ(row->Type(), ot::blockchain::Type::Bitcoin);
        }

        EXPECT_FALSE(row->Last());
    }
}

TEST_F(Test_BlockchainSelector, shutdown)
{
    EXPECT_EQ(counter_full_.expected_, counter_full_.updated_);
    EXPECT_EQ(counter_main_.expected_, counter_main_.updated_);
    EXPECT_EQ(counter_test_.expected_, counter_test_.updated_);
}
}  // namespace ottest
