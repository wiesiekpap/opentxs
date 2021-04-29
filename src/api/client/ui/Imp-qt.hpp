// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>

#include "api/client/ui/Imp-base.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/ui/qt/AccountActivity.hpp"
#include "opentxs/ui/qt/AccountList.hpp"
#include "opentxs/ui/qt/AccountSummary.hpp"
#include "opentxs/ui/qt/ActivitySummary.hpp"
#include "opentxs/ui/qt/ActivityThread.hpp"
#include "opentxs/ui/qt/BlankModel.hpp"
#include "opentxs/ui/qt/BlockchainSelection.hpp"
#include "opentxs/ui/qt/Contact.hpp"
#include "opentxs/ui/qt/ContactList.hpp"
#include "opentxs/ui/qt/MessagableList.hpp"
#include "opentxs/ui/qt/PayableList.hpp"
#include "opentxs/ui/qt/Profile.hpp"
#include "opentxs/ui/qt/SeedValidator.hpp"
#include "opentxs/ui/qt/UnitList.hpp"

class QAbstractItemModel;

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
struct Manager;
}  // namespace internal
}  // namespace client
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

class Flag;
}  // namespace opentxs

namespace opentxs::api::client::ui
{
class ImpQt final : public UI::Imp
{
public:
    auto AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountActivityQt* final;
    auto AccountListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::AccountListQt* final;
    auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountSummaryQt* final;
    auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivitySummaryQt* final;
    auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivityThreadQt* final;
    auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel* final;
    auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainSelectionQt* final;
    auto ContactQt(const Identifier& contactID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ContactQt* final;
    auto ContactListQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ContactListQt* final;
    auto MessagableListQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::MessagableListQt* final;
    auto PayableListQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::PayableListQt* final;
    auto ProfileQt(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ProfileQt* final;
    auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator* final;
    auto UnitListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::UnitListQt* final;

    auto ShutdownModels() noexcept -> void final;

    ImpQt(
        const api::client::internal::Manager& api,
        const api::client::internal::Blockchain& blockchain,
        const Flag& running) noexcept;

    ~ImpQt() final;

private:
    using AccountActivityQtValue =
        std::unique_ptr<opentxs::ui::AccountActivityQt>;
    using AccountListQtValue = std::unique_ptr<opentxs::ui::AccountListQt>;
    using AccountSummaryQtValue =
        std::unique_ptr<opentxs::ui::AccountSummaryQt>;
    using ActivitySummaryQtValue =
        std::unique_ptr<opentxs::ui::ActivitySummaryQt>;
    using ActivityThreadQtValue =
        std::unique_ptr<opentxs::ui::ActivityThreadQt>;
    using ContactQtValue = std::unique_ptr<opentxs::ui::ContactQt>;
    using ContactListQtValue = std::unique_ptr<opentxs::ui::ContactListQt>;
    using MessagableListQtValue =
        std::unique_ptr<opentxs::ui::MessagableListQt>;
    using PayableListQtValue = std::unique_ptr<opentxs::ui::PayableListQt>;
    using ProfileQtValue = std::unique_ptr<opentxs::ui::ProfileQt>;
    using UnitListQtValue = std::unique_ptr<opentxs::ui::UnitListQt>;
    using AccountActivityQtMap =
        std::map<AccountActivityKey, AccountActivityQtValue>;
    using AccountListQtMap = std::map<AccountListKey, AccountListQtValue>;
    using AccountSummaryQtMap =
        std::map<AccountSummaryKey, AccountSummaryQtValue>;
    using ActivitySummaryQtMap =
        std::map<ActivitySummaryKey, ActivitySummaryQtValue>;
    using ActivityThreadQtMap =
        std::map<ActivityThreadKey, ActivityThreadQtValue>;
    using ContactQtMap = std::map<ContactKey, ContactQtValue>;
    using ContactListQtMap = std::map<ContactListKey, ContactListQtValue>;
    using MessagableListQtMap =
        std::map<MessagableListKey, MessagableListQtValue>;
    using PayableListQtMap = std::map<PayableListKey, PayableListQtValue>;
    using ProfileQtMap = std::map<ProfileKey, ProfileQtValue>;
    using SeedValidatorMap = std::map<
        opentxs::crypto::SeedStyle,
        std::map<opentxs::crypto::Language, opentxs::ui::SeedValidator>>;
    using UnitListQtMap = std::map<UnitListKey, UnitListQtValue>;
    using BlockchainSelectionQtPointer =
        std::unique_ptr<opentxs::ui::BlockchainSelectionQt>;
    using BlockchainSelectionQtType =
        std::map<opentxs::ui::Blockchains, BlockchainSelectionQtPointer>;

    struct Blank {
        auto get(const std::size_t columns) noexcept
            -> opentxs::ui::BlankModel*;

    private:
        std::mutex lock_{};
        std::map<std::size_t, opentxs::ui::BlankModel> map_{};
    };

    mutable Blank blank_;
    mutable AccountActivityQtMap accounts_qt_;
    mutable AccountListQtMap account_lists_qt_;
    mutable AccountSummaryQtMap account_summaries_qt_;
    mutable ActivitySummaryQtMap activity_summaries_qt_;
    mutable ContactQtMap contacts_qt_;
    mutable ContactListQtMap contact_lists_qt_;
    mutable MessagableListQtMap messagable_lists_qt_;
    mutable PayableListQtMap payable_lists_qt_;
    mutable ActivityThreadQtMap activity_threads_qt_;
    mutable ProfileQtMap profiles_qt_;
    mutable SeedValidatorMap seed_validators_;
    mutable UnitListQtMap unit_lists_qt_;
    mutable BlockchainSelectionQtType blockchain_selection_qt_;

    ImpQt() = delete;
    ImpQt(const ImpQt&) = delete;
    ImpQt(ImpQt&&) = delete;
    auto operator=(const ImpQt&) -> ImpQt& = delete;
    auto operator=(ImpQt&&) -> ImpQt& = delete;
};
}  // namespace opentxs::api::client::ui
