// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "internal/api/client/Client.hpp"  // IWYU pragma: associated
#include "opentxs/api/client/UI.hpp"       // IWYU pragma: associated

#include <memory>

#include "api/client/ui/Imp-base.hpp"

// #define OT_METHOD "opentxs::api::UI"

namespace opentxs::api::client
{
UI::UI(Imp* imp) noexcept
    : imp_(imp)
{
    // WARNING: do not access api_.Wallet() during construction
}

auto UI::AccountActivity(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::AccountActivity&
{
    return imp_->AccountActivity(nymID, accountID, cb);
}

auto UI::AccountActivityQt(
    const identifier::Nym& nymID,
    const Identifier& accountID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::AccountActivityQt*
{
    return imp_->AccountActivityQt(nymID, accountID, cb);
}

auto UI::AccountList(const identifier::Nym& nym, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::AccountList&
{
    return imp_->AccountList(nym, cb);
}

auto UI::AccountListQt(const identifier::Nym& nym, const SimpleCallback cb)
    const noexcept -> opentxs::ui::AccountListQt*
{
    return imp_->AccountListQt(nym, cb);
}

auto UI::AccountSummary(
    const identifier::Nym& nymID,
    const contact::ContactItemType currency,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::AccountSummary&
{
    return imp_->AccountSummary(nymID, currency, cb);
}

auto UI::AccountSummaryQt(
    const identifier::Nym& nymID,
    const contact::ContactItemType currency,
    const SimpleCallback cb) const noexcept -> opentxs::ui::AccountSummaryQt*
{
    return imp_->AccountSummaryQt(nymID, currency, cb);
}

auto UI::ActivitySummary(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::ActivitySummary&
{
    return imp_->ActivitySummary(nymID, cb);
}

auto UI::ActivitySummaryQt(
    const identifier::Nym& nymID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::ActivitySummaryQt*
{
    return imp_->ActivitySummaryQt(nymID, cb);
}

auto UI::ActivityThread(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::ActivityThread&
{
    return imp_->ActivityThread(nymID, threadID, cb);
}

auto UI::ActivityThreadQt(
    const identifier::Nym& nymID,
    const Identifier& threadID,
    const SimpleCallback cb) const noexcept -> opentxs::ui::ActivityThreadQt*
{
    return imp_->ActivityThreadQt(nymID, threadID, cb);
}

auto UI::BlankModel(const std::size_t columns) const noexcept
    -> QAbstractItemModel*
{
    return imp_->BlankModel(columns);
}

auto UI::BlockchainAccountStatus(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain,
    const SimpleCallback cb) const noexcept
    -> const opentxs::ui::BlockchainAccountStatus&
{
    return imp_->BlockchainAccountStatus(nymID, chain, cb);
}

auto UI::BlockchainAccountStatusQt(
    const identifier::Nym& nymID,
    const opentxs::blockchain::Type chain,
    const SimpleCallback cb) const noexcept
    -> opentxs::ui::BlockchainAccountStatusQt*
{
    return imp_->BlockchainAccountStatusQt(nymID, chain, cb);
}

auto UI::BlockchainIssuerID(const opentxs::blockchain::Type chain)
    const noexcept -> const identifier::Nym&
{
    return imp_->BlockchainIssuerID(chain);
}

auto UI::BlockchainNotaryID(const opentxs::blockchain::Type chain)
    const noexcept -> const identifier::Server&
{
    return imp_->BlockchainNotaryID(chain);
}

auto UI::BlockchainSelection(
    const opentxs::ui::Blockchains type,
    const SimpleCallback updateCB) const noexcept
    -> const opentxs::ui::BlockchainSelection&
{
    return imp_->BlockchainSelection(type, updateCB);
}

auto UI::BlockchainSelectionQt(
    const opentxs::ui::Blockchains type,
    const SimpleCallback updateCB) const noexcept
    -> opentxs::ui::BlockchainSelectionQt*
{
    return imp_->BlockchainSelectionQt(type, updateCB);
}

auto UI::BlockchainStatistics(const SimpleCallback updateCB) const noexcept
    -> const opentxs::ui::BlockchainStatistics&
{
    return imp_->BlockchainStatistics(updateCB);
}

auto UI::BlockchainStatisticsQt(const SimpleCallback updateCB) const noexcept
    -> opentxs::ui::BlockchainStatisticsQt*
{
    return imp_->BlockchainStatisticsQt(updateCB);
}

auto UI::BlockchainUnitID(const opentxs::blockchain::Type chain) const noexcept
    -> const identifier::UnitDefinition&
{
    return imp_->BlockchainUnitID(chain);
}

auto UI::Contact(const Identifier& contactID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::Contact&
{
    return imp_->Contact(contactID, cb);
}

auto UI::ContactQt(const Identifier& contactID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::ContactQt*
{
    return imp_->ContactQt(contactID, cb);
}

auto UI::ContactList(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::ContactList&
{
    return imp_->ContactList(nymID, cb);
}

auto UI::ContactListQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::ContactListQt*
{
    return imp_->ContactListQt(nymID, cb);
}

auto UI::MessagableList(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::MessagableList&
{
    return imp_->MessagableList(nymID, cb);
}

auto UI::MessagableListQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::MessagableListQt*
{
    return imp_->MessagableListQt(nymID, cb);
}

auto UI::PayableList(
    const identifier::Nym& nymID,
    const contact::ContactItemType currency,
    const SimpleCallback cb) const noexcept -> const opentxs::ui::PayableList&
{
    return imp_->PayableList(nymID, currency, cb);
}

auto UI::PayableListQt(
    const identifier::Nym& nymID,
    const contact::ContactItemType currency,
    const SimpleCallback cb) const noexcept -> opentxs::ui::PayableListQt*
{
    return imp_->PayableListQt(nymID, currency, cb);
}

auto UI::Profile(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::Profile&
{
    return imp_->Profile(nymID, cb);
}

auto UI::ProfileQt(const identifier::Nym& nymID, const SimpleCallback cb)
    const noexcept -> opentxs::ui::ProfileQt*
{
    return imp_->ProfileQt(nymID, cb);
}

auto UI::SeedValidator(
    const opentxs::crypto::SeedStyle type,
    const opentxs::crypto::Language lang) const noexcept
    -> const opentxs::ui::SeedValidator*
{
    return imp_->SeedValidator(type, lang);
}

auto UI::UnitList(const identifier::Nym& nym, const SimpleCallback cb)
    const noexcept -> const opentxs::ui::UnitList&
{
    return imp_->UnitList(nym, cb);
}

auto UI::UnitListQt(const identifier::Nym& nym, const SimpleCallback cb)
    const noexcept -> opentxs::ui::UnitListQt*
{
    return imp_->UnitListQt(nym, cb);
}

UI::~UI() { std::unique_ptr<Imp>{imp_}.reset(); }
}  // namespace opentxs::api::client

namespace opentxs::api::client::internal
{
UI::UI(client::UI::Imp* imp) noexcept
    : client::UI(imp)
{
    // WARNING: do not access api_.Wallet() during construction
}

auto UI::ActivateUICallback(const Identifier& widget) const noexcept -> void
{
    imp_->ActivateUICallback(widget);
}

auto UI::ClearUICallbacks(const Identifier& widget) const noexcept -> void
{
    imp_->ClearUICallbacks(widget);
}

auto UI::Init() noexcept -> void { imp_->Init(); }

auto UI::RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
    const noexcept -> void
{
    imp_->RegisterUICallback(widget, cb);
}

auto UI::Shutdown() noexcept -> void { imp_->Shutdown(); }
}  // namespace opentxs::api::client::internal
