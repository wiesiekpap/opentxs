// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_CLIENT_UI_HPP
#define OPENTXS_API_CLIENT_UI_HPP

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/contact/ContactItemType.hpp"
// IWYU pragma: no_include "opentxs/ui/Blockchains.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <iosfwd>

#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/ui/Types.hpp"

#ifdef SWIG
// clang-format off
%extend opentxs::api::client::UI {
    const opentxs::ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const int currency) const noexcept
    {
        return $self->AccountSummary(
            nymID,
            static_cast<opentxs::contact::ContactItemType>(currency));
    }
    const opentxs::ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const int currency) const noexcept
    {
        return $self->PayableList(
            nymID,
            static_cast<opentxs::contact::ContactItemType>(currency));
    }
}
%ignore opentxs::api::client::UI::AccountSummary;
%ignore opentxs::api::client::UI::PayableList;
// clang-format on
#endif  // SWIG

class QAbstractItemModel;

namespace opentxs
{
namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace ui
{
class AccountActivity;
class AccountActivityQt;
class AccountList;
class AccountListQt;
class AccountSummary;
class AccountSummaryQt;
class ActivitySummary;
class ActivitySummaryQt;
class ActivityThread;
class ActivityThreadQt;
class BlockchainAccountStatus;
class BlockchainAccountStatusQt;
class BlockchainSelection;
class BlockchainSelectionQt;
class BlockchainStatistics;
class BlockchainStatisticsQt;
class Contact;
class ContactList;
class ContactListQt;
class ContactQt;
class MessagableList;
class MessagableListQt;
class PayableList;
class PayableListQt;
class Profile;
class ProfileQt;
class SeedValidator;
class UnitList;
class UnitListQt;
}  // namespace ui

class Identifier;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
namespace client
{
class OPENTXS_EXPORT UI
{
public:
    struct Imp;

    const opentxs::ui::AccountActivity& AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::AccountActivityQt* AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::AccountList& AccountList(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::AccountListQt* AccountListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::AccountSummary& AccountSummary(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::AccountSummaryQt* AccountSummaryQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::ActivitySummary& ActivitySummary(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::ActivitySummaryQt* ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::ActivityThread& ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::ActivityThreadQt* ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    QAbstractItemModel* BlankModel(const std::size_t columns) const noexcept;
    const opentxs::ui::BlockchainAccountStatus& BlockchainAccountStatus(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::BlockchainAccountStatusQt* BlockchainAccountStatusQt(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB = {}) const noexcept;
    const identifier::Nym& BlockchainIssuerID(
        const opentxs::blockchain::Type chain) const noexcept;
    const identifier::Server& BlockchainNotaryID(
        const opentxs::blockchain::Type chain) const noexcept;
    const opentxs::ui::BlockchainSelection& BlockchainSelection(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::BlockchainSelectionQt* BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::BlockchainStatistics& BlockchainStatistics(
        const SimpleCallback updateCB = {}) const noexcept;
    opentxs::ui::BlockchainStatisticsQt* BlockchainStatisticsQt(
        const SimpleCallback updateCB = {}) const noexcept;
    const identifier::UnitDefinition& BlockchainUnitID(
        const opentxs::blockchain::Type chain) const noexcept;
    const opentxs::ui::Contact& Contact(
        const Identifier& contactID,
        const SimpleCallback updateCB = {}) const noexcept;
    opentxs::ui::ContactQt* ContactQt(
        const Identifier& contactID,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::ContactList& ContactList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::ContactListQt* ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::MessagableList& MessagableList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::MessagableListQt* MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::PayableList& PayableList(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::PayableListQt* PayableListQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept;
    const opentxs::ui::Profile& Profile(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::ProfileQt* ProfileQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    const opentxs::ui::SeedValidator* SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept;
    const opentxs::ui::UnitList& UnitList(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept;
    /// Caller does not own this pointer
    opentxs::ui::UnitListQt* UnitListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept;

    OPENTXS_NO_EXPORT virtual ~UI();

    OPENTXS_NO_EXPORT UI(Imp*) noexcept;

protected:
    Imp* imp_;

private:
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
