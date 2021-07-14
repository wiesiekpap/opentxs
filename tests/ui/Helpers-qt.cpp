// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <QObject>
#include <QString>
#include <QVariant>
#include <cstddef>

#include "integration/Helpers.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/qt/ContactList.hpp"

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

    EXPECT_EQ(widget.columnCount(), 1);
    EXPECT_EQ(widget.rowCount(), v.size());

    auto output{true};
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
}  // namespace ottest
