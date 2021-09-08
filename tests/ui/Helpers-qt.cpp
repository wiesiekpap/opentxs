// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <QAbstractItemModel>
#include <QObject>
#include <QString>
#include <QVariant>
#include <cstddef>

#include "Basic.hpp"
#include "integration/Helpers.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/qt/BlockchainAccountStatus.hpp"
#include "opentxs/ui/qt/ContactList.hpp"

namespace ottest
{
auto check_qt_common(const QAbstractItemModel& model) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountSourceData& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountData& expected,
    const int row) noexcept -> bool;
auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubchainData& expected,
    const int row) noexcept -> bool;
}  // namespace ottest

namespace ottest
{
auto check_account_activity_qt(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool
{
    return true;  // TODO
}

auto check_activity_thread_qt(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool
{
    return true;  // TODO
}

auto check_blockchain_account_status_qt(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool
{
    const auto* pModel =
        user.api_->UI().BlockchainAccountStatusQt(user.nym_id_, chain);

    EXPECT_NE(pModel, nullptr);

    if (nullptr == pModel) { return false; }

    const auto& model = *pModel;
    auto output = check_qt_common(model);
    auto parent = QModelIndex{};
    const auto vCount = expected.rows_.size();

    output &= (model.getNym().toStdString() == expected.owner_);
    output &=
        (static_cast<ot::blockchain::Type>(model.getChain()) ==
         expected.chain_);
    output &= (model.columnCount(parent) == 1);
    output &= (static_cast<std::size_t>(model.rowCount(parent)) == vCount);

    EXPECT_EQ(model.getNym().toStdString(), expected.owner_);
    EXPECT_EQ(
        static_cast<ot::blockchain::Type>(model.getChain()), expected.chain_);
    EXPECT_EQ(model.columnCount(parent), 1);
    EXPECT_EQ(model.rowCount(parent), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = std::size_t{}; i < vCount; ++i, ++it) {
        output &= check_row(model, parent, *it, static_cast<int>(i));
    }

    return output;
}

auto check_contact_list_qt(
    const User& user,
    const std::vector<ContactListData>& v) noexcept -> bool
{
    EXPECT_GT(v.size(), 0);

    if (0 == v.size()) { return false; }

    const auto* p = user.api_->UI().ContactListQt(user.nym_id_);

    EXPECT_NE(p, nullptr);

    if (nullptr == p) { return false; }

    const auto& widget = *p;
    auto output = check_qt_common(widget);

    output &= (widget.columnCount() == 1);
    output &= (widget.rowCount() == static_cast<int>(v.size()));

    EXPECT_EQ(widget.columnCount(), 1);
    EXPECT_EQ(widget.rowCount(), static_cast<int>(v.size()));

    auto it{v.begin()};

    for (auto i = std::size_t{}; i < v.size(); ++i, ++it) {
        const auto& expect = *it;
        const auto row = static_cast<int>(i);
        const auto exists = widget.hasIndex(row, 0);

        EXPECT_TRUE(exists);

        if (false == exists) { continue; }

        const auto index = widget.index(row, 0);
        using Model = ot::ui::ContactListQt;
        const auto id = widget.data(index, Model::IDRole);
        const auto name = widget.data(index, Model::NameRole);
        const auto section = widget.data(index, Model::SectionRole);
        const auto display = widget.data(index, Qt::DisplayRole);
        const auto& cIndex = expect.contact_id_index_;

        if (auto required = user.Contact(cIndex).str(); required.empty()) {
            const auto set =
                user.SetContact(cIndex, id.toString().toStdString());
            required = user.Contact(cIndex).str();
            output &= set;
            output &= (false == required.empty());

            EXPECT_TRUE(set);
            EXPECT_FALSE(required.empty());
        } else {
            const auto actual = id.toString().toStdString();
            output &= (required == actual);

            EXPECT_EQ(actual, required);
        }

        {
            const auto required = expect.name_;
            const auto actual = name.toString().toStdString();
            output &= (required == actual);

            EXPECT_EQ(actual, required);
        }

        {
            const auto required = expect.section_;
            const auto actual = section.toString().toStdString();
            output &= (required == actual);

            EXPECT_EQ(actual, required);
        }

        output &= (display == name);

        EXPECT_EQ(display, name);
    }

    return output;
}

auto check_qt_common(const QAbstractItemModel& model) noexcept -> bool
{
    const auto* root = GetQT();

    EXPECT_NE(root, nullptr);

    if (nullptr == root) { return false; }

    auto output{true};
    output &= (model.thread() == root->thread());

    EXPECT_EQ(model.thread(), root->thread());

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountSourceData& expected,
    const int row) noexcept -> bool
{
    const auto exists = model.hasIndex(row, 0, parent);

    EXPECT_TRUE(exists);

    if (false == exists) { return false; }

    using Model = ot::ui::BlockchainAccountStatusQt;
    auto output{true};
    const auto vCount = expected.rows_.size();
    const auto index = model.index(row, 0, parent);
    const auto name = model.data(index, Model::NameRole);
    const auto id = model.data(index, Model::SourceIDRole);
    const auto type = model.data(index, Model::SubaccountTypeRole);

    output &= (name.toString().toStdString() == expected.name_);
    output &= (id.toString().toStdString() == expected.id_);
    output &=
        (static_cast<ot::blockchain::crypto::SubaccountType>(type.toInt()) ==
         expected.type_);
    output &= (model.columnCount(index) == 1);
    output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

    EXPECT_EQ(name.toString().toStdString(), expected.name_);
    EXPECT_EQ(id.toString().toStdString(), expected.id_);
    EXPECT_EQ(
        static_cast<ot::blockchain::crypto::SubaccountType>(type.toInt()),
        expected.type_);
    EXPECT_EQ(model.columnCount(index), 1);
    EXPECT_EQ(model.rowCount(index), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = std::size_t{}; i < vCount; ++i, ++it) {
        output &= check_row(model, index, *it, static_cast<int>(i));
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubaccountData& expected,
    const int row) noexcept -> bool
{
    const auto exists = model.hasIndex(row, 0, parent);

    EXPECT_TRUE(exists);

    if (false == exists) { return false; }

    using Model = ot::ui::BlockchainAccountStatusQt;
    auto output{true};
    const auto vCount = expected.rows_.size();
    const auto index = model.index(row, 0, parent);
    const auto name = model.data(index, Model::NameRole);
    const auto id = model.data(index, Model::SubaccountIDRole);

    output &= (name.toString().toStdString() == expected.name_);
    output &= (id.toString().toStdString() == expected.id_);
    output &= (model.columnCount(index) == 1);
    output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

    EXPECT_EQ(name.toString().toStdString(), expected.name_);
    EXPECT_EQ(id.toString().toStdString(), expected.id_);
    EXPECT_EQ(model.columnCount(index), 1);
    EXPECT_EQ(model.rowCount(index), vCount);

    if (0u == vCount) { return output; }

    auto it{expected.rows_.begin()};

    for (auto i = std::size_t{}; i < vCount; ++i, ++it) {
        output &= check_row(model, index, *it, static_cast<int>(i));
    }

    return output;
}

auto check_row(
    const QAbstractItemModel& model,
    const QModelIndex& parent,
    const BlockchainSubchainData& expected,
    const int row) noexcept -> bool
{
    const auto exists = model.hasIndex(row, 0, parent);

    EXPECT_TRUE(exists);

    if (false == exists) { return false; }

    using Model = ot::ui::BlockchainAccountStatusQt;
    auto output{true};
    const auto vCount = 0;
    const auto index = model.index(row, 0, parent);
    const auto name = model.data(index, Model::NameRole);
    const auto type = model.data(index, Model::SubchainTypeRole);

    output &= (name.toString().toStdString() == expected.name_);
    output &=
        (static_cast<ot::blockchain::crypto::Subchain>(type.toInt()) ==
         expected.type_);
    output &= (model.columnCount(index) == 1);
    output &= (static_cast<std::size_t>(model.rowCount(index)) == vCount);

    EXPECT_EQ(name.toString().toStdString(), expected.name_);
    EXPECT_EQ(
        static_cast<ot::blockchain::crypto::Subchain>(type.toInt()),
        expected.type_);
    EXPECT_EQ(model.columnCount(index), 1);
    EXPECT_EQ(model.rowCount(index), vCount);

    return output;
}
}  // namespace ottest
