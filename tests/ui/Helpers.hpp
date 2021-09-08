// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/ContactItemType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/Subchain.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"

#pragma once

#include <atomic>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Basic.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
class Identifier;
}  // namespace opentxs

namespace ottest
{
struct User;

struct Counter {
    std::atomic_int expected_{};
    std::atomic_int updated_{};
};

struct AccountActivityRow {
    ot::StorageBox type_{};
    ot::Amount amount_{};
    std::string display_amount_{};
    std::vector<std::string> contacts_{};
    std::string memo_{};
    std::string workflow_{};
    std::string text_{};
    std::string uuid_{};
    std::optional<ot::Time> timestamp_{};
};

struct AccountActivityData {
    ot::AccountType type_;
    std::string id_{};
    ot::contact::ContactItemType unit_;
    std::string contract_id_{};
    std::string contract_name_{};
    std::string notary_id_{};
    std::string notary_name{};
    int polarity_{};
    ot::Amount balance_{};
    std::string display_balance_{};
    std::string default_deposit_address_{};
    std::map<ot::blockchain::Type, std::string> deposit_addresses_{};
    double sync_{};
    std::pair<int, int> progress_{};
    std::vector<std::pair<std::string, bool>> addresses_to_validate_{};
    std::vector<std::pair<std::string, std::string>> amounts_to_validate_{};
    std::vector<AccountActivityRow> rows_{};
};

struct ActivityThreadRow {
    bool loading_{};
    bool pending_{};
    bool outgoing_{};
    ot::Amount amount_{};
    std::string display_amount_{};
    std::string from_{};
    std::string text_{};
    std::string memo_{};
    ot::StorageBox type_{};
    std::optional<ot::Time> timestamp_{};
};

struct ActivityThreadData {
    bool can_message_{};
    std::string thread_id_{};
    std::string display_name_{};
    std::string draft_{};
    std::string participants_{};
    std::map<ot::contact::ContactItemType, std::string> payment_codes_{};
    std::vector<ActivityThreadRow> rows_{};
};

struct BlockchainSubchainData {
    std::string name_;
    ot::blockchain::crypto::Subchain type_;
};

struct BlockchainSubaccountData {
    std::string name_;
    std::string id_;
    std::vector<BlockchainSubchainData> rows_;
};

struct BlockchainSubaccountSourceData {
    std::string name_;
    std::string id_;
    ot::blockchain::crypto::SubaccountType type_;
    std::vector<BlockchainSubaccountData> rows_;
};

struct BlockchainAccountStatusData {
    std::string owner_;
    ot::blockchain::Type chain_;
    std::vector<BlockchainSubaccountSourceData> rows_;
};

struct ContactListData {
    bool check_contact_id_{};
    std::string contact_id_index_{};
    std::string name_{};
    std::string section_{};
    std::string image_{};
};

auto check_account_activity(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool;
auto check_account_activity_qt(
    const User& user,
    const ot::Identifier& account,
    const AccountActivityData& expected) noexcept -> bool;

auto check_activity_thread(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool;
auto check_activity_thread_qt(
    const User& user,
    const ot::Identifier& contact,
    const ActivityThreadData& expected) noexcept -> bool;

auto check_blockchain_account_status(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;
auto check_blockchain_account_status_qt(
    const User& user,
    const ot::blockchain::Type chain,
    const BlockchainAccountStatusData& expected) noexcept -> bool;

auto check_contact_list(
    const User& user,
    const std::vector<ContactListData>& expected) noexcept -> bool;
auto check_contact_list_qt(
    const User& user,
    const std::vector<ContactListData>& expected) noexcept -> bool;

auto make_cb(Counter& counter, const std::string name) noexcept
    -> std::function<void()>;

auto wait_for_counter(Counter& data, const bool hard = true) noexcept -> bool;
}  // namespace ottest
