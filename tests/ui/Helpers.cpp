// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <iterator>

#include "integration/Helpers.hpp"
#include "opentxs/SharedPimpl.hpp"
#include "opentxs/api/client/Manager.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BlockchainAccountStatus.hpp"
#include "opentxs/ui/BlockchainSubaccount.hpp"
#include "opentxs/ui/BlockchainSubaccountSource.hpp"
#include "opentxs/ui/BlockchainSubchain.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/ContactListItem.hpp"

namespace ottest
{
auto check_blockchain_subaccounts(
    const ot::ui::BlockchainSubaccountSource& widget,
    const std::vector<BlockchainSubaccountData>& v) noexcept -> bool;
auto check_blockchain_subchains(
    const ot::ui::BlockchainSubaccount& widget,
    const std::vector<BlockchainSubchainData>& v) noexcept -> bool;

auto check_account_activity(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool
{
    return true;  // TODO
}

auto check_activity_thread(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool
{
    const auto& widget = user.api_->UI().ActivityThread(user.nym_id_, contact);
    auto output{true};
    output &= (widget.CanMessage() == expected.can_message_);
    output &= (widget.DisplayName() == expected.display_name_);
    output &= (widget.GetDraft() == expected.draft_);
    output &= (widget.Participants() == expected.participants_);
    output &= (widget.ThreadID() == expected.thread_id_);

    EXPECT_EQ(widget.CanMessage(), expected.can_message_);
    EXPECT_EQ(widget.DisplayName(), expected.display_name_);
    EXPECT_EQ(widget.GetDraft(), expected.draft_);
    EXPECT_EQ(widget.Participants(), expected.participants_);
    EXPECT_EQ(widget.ThreadID(), expected.thread_id_);

    for (const auto& [type, required] : expected.payment_codes_) {
        const auto existing = widget.PaymentCode(type);

        output &= (existing == required);

        EXPECT_EQ(existing, required);
    }

    const auto& v = expected.rows_;
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        output &= (row->Loading() == it->loading_);
        output &= (row->Pending() == it->pending_);
        output &= (row->Amount() == it->amount_);
        output &= (row->DisplayAmount() == it->display_amount_);
        output &= (row->From() == it->from_);
        output &= (row->Memo() == it->memo_);
        output &= (row->Outgoing() == it->outgoing_);
        output &= (row->Text() == it->text_);
        output &= (row->Type() == it->type_);

        EXPECT_EQ(row->Loading(), it->loading_);
        EXPECT_EQ(row->Pending(), it->pending_);
        EXPECT_EQ(row->Amount(), it->amount_);
        EXPECT_EQ(row->DisplayAmount(), it->display_amount_);
        EXPECT_EQ(row->From(), it->from_);
        EXPECT_EQ(row->Memo(), it->memo_);
        EXPECT_EQ(row->Outgoing(), it->outgoing_);
        EXPECT_EQ(row->Text(), it->text_);
        EXPECT_EQ(row->Type(), it->type_);

        if (it->timestamp_.has_value()) {
            const auto& time = it->timestamp_.value();

            output &= (row->Timestamp() == time);

            EXPECT_EQ(row->Timestamp(), time);
        } else {
            static const auto null = ot::Time{};

            output &= (row->Timestamp() != null);

            EXPECT_NE(row->Timestamp(), null);
        }

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool
{
    const auto& widget =
        user.api_->UI().BlockchainAccountStatus(user.nym_id_, chain);
    auto output{true};
    output &= (widget.Chain() == expected.chain_);
    output &= (widget.Owner().str() == expected.owner_);

    EXPECT_EQ(widget.Chain(), expected.chain_);
    EXPECT_EQ(widget.Owner().str(), expected.owner_);

    const auto& v = expected.rows_;
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        output &= (row->Name() == it->name_);
        output &= (row->SourceID().str() == it->id_);
        output &= (row->Type() == it->type_);
        output &= check_blockchain_subaccounts(row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SourceID().str(), it->id_);
        EXPECT_EQ(row->Type(), it->type_);

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto check_blockchain_subaccounts(
    const ot::ui::BlockchainSubaccountSource& widget,
    const std::vector<BlockchainSubaccountData>& v) noexcept -> bool
{
    auto output{true};
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        output &= (row->Name() == it->name_);
        output &= (row->SubaccountID().str() == it->id_);
        output &= check_blockchain_subchains(row.get(), it->rows_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->SubaccountID().str(), it->id_);

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto check_blockchain_subchains(
    const ot::ui::BlockchainSubaccount& widget,
    const std::vector<BlockchainSubchainData>& v) noexcept -> bool
{
    auto output{true};
    auto row = widget.First();

    if (const auto valid = row->Valid(); 0 < v.size()) {
        output &= valid;

        EXPECT_TRUE(valid);

        if (false == valid) { return output; }
    } else {
        output &= (false == valid);

        EXPECT_FALSE(valid);
    }

    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        output &= (row->Name() == it->name_);
        output &= (row->Type() == it->type_);

        EXPECT_EQ(row->Name(), it->name_);
        EXPECT_EQ(row->Type(), it->type_);

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto check_contact_list(
    const User& user,
    const std::vector<ContactListData>& v) noexcept -> bool
{
    EXPECT_GT(v.size(), 0);

    if (0 == v.size()) { return false; }

    const auto& widget = user.api_->UI().ContactList(user.nym_id_);
    auto output{true};
    auto row = widget.First();

    EXPECT_TRUE(row->Valid());

    if (false == row->Valid()) { return false; }

    // NOTE contact list is different than other widgets because it is required
    // to always have one row. Other widgets are allowed to be empty.
    for (auto it{v.begin()}; it < v.end(); ++it, row = widget.Next()) {
        const auto& index = it->contact_id_index_;

        if (it->check_contact_id_) {
            const auto match = (row->ContactID() == user.Contact(index).str());

            output &= match;

            EXPECT_EQ(row->ContactID(), user.Contact(index).str());
        } else {
            const auto set = user.SetContact(index, row->ContactID());
            const auto exists = (false == user.Contact(index).empty());

            output &= set;
            output &= exists;

            EXPECT_TRUE(set);
            EXPECT_TRUE(exists);
        }

        output &= (row->DisplayName() == it->name_);

        EXPECT_EQ(row->DisplayName(), it->name_);

        output &= (row->ImageURI() == it->image_);

        EXPECT_EQ(row->ImageURI(), it->image_);

        output &= (row->Section() == it->section_);

        EXPECT_EQ(row->Section(), it->section_);

        const auto lastVector = std::next(it) == v.end();
        const auto lastRow = row->Last();
        output &= (lastVector == lastRow);

        if (lastVector) {
            EXPECT_TRUE(lastRow);
        } else {
            EXPECT_FALSE(lastRow);

            if (lastRow) { return output; }
        }
    }

    return output;
}

auto make_cb(Counter& counter, const std::string name) noexcept
    -> std::function<void()>
{
    return [&counter, name]() {
        auto& [expected, value] = counter;

        if (++value > expected) { std::cout << name << ": " << value << '\n'; }
    };
}

auto wait_for_counter(Counter& data, const bool hard) noexcept -> bool
{
    const auto limit =
        hard ? std::chrono::seconds(300) : std::chrono::seconds(10);
    auto start = ot::Clock::now();
    auto& [expected, updated] = data;

    while ((updated < expected) && ((ot::Clock::now() - start) < limit)) {
        ot::Sleep(std::chrono::milliseconds(100));
    }

    if (false == hard) { updated.store(expected.load()); }

    return updated >= expected;
}
}  // namespace ottest
