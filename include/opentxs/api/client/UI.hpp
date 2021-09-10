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

    auto AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountActivity&;
    /// Caller does not own this pointer
    auto AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountActivityQt*;
    auto AccountList(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountList&;
    /// Caller does not own this pointer
    auto AccountListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountListQt*;
    auto AccountSummary(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::AccountSummary&;
    /// Caller does not own this pointer
    auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::AccountSummaryQt*;
    auto ActivitySummary(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::ActivitySummary&;
    /// Caller does not own this pointer
    auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ActivitySummaryQt*;
    auto ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::ActivityThread&;
    /// Caller does not own this pointer
    auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ActivityThreadQt*;
    /// Caller does not own this pointer
    auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel*;
    auto BlockchainAccountStatus(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::BlockchainAccountStatus&;
    /// Caller does not own this pointer
    auto BlockchainAccountStatusQt(
        const identifier::Nym& nymID,
        const opentxs::blockchain::Type chain,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::BlockchainAccountStatusQt*;
    auto BlockchainIssuerID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Nym&;
    auto BlockchainNotaryID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Server&;
    auto BlockchainSelection(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::BlockchainSelection&;
    /// Caller does not own this pointer
    auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::BlockchainSelectionQt*;
    auto BlockchainStatistics(const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::BlockchainStatistics&;
    auto BlockchainStatisticsQt(const SimpleCallback updateCB = {})
        const noexcept -> opentxs::ui::BlockchainStatisticsQt*;
    auto BlockchainUnitID(const opentxs::blockchain::Type chain) const noexcept
        -> const identifier::UnitDefinition&;
    auto Contact(
        const Identifier& contactID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::Contact&;
    auto ContactQt(
        const Identifier& contactID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ContactQt*;
    auto ContactList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::ContactList&;
    /// Caller does not own this pointer
    auto ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ContactListQt*;
    auto MessagableList(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::MessagableList&;
    /// Caller does not own this pointer
    auto MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::MessagableListQt*;
    auto PayableList(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::PayableList&;
    /// Caller does not own this pointer
    auto PayableListQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::PayableListQt*;
    auto Profile(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::Profile&;
    /// Caller does not own this pointer
    auto ProfileQt(
        const identifier::Nym& nymID,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::ProfileQt*;
    /// Caller does not own this pointer
    auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator*;
    auto UnitList(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> const opentxs::ui::UnitList&;
    /// Caller does not own this pointer
    auto UnitListQt(
        const identifier::Nym& nym,
        const SimpleCallback updateCB = {}) const noexcept
        -> opentxs::ui::UnitListQt*;

    OPENTXS_NO_EXPORT virtual ~UI();

    OPENTXS_NO_EXPORT UI(Imp*) noexcept;

protected:
    Imp* imp_;

private:
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    auto operator=(const UI&) -> UI& = delete;
    auto operator=(UI&&) -> UI& = delete;
};
}  // namespace client
}  // namespace api
}  // namespace opentxs
#endif
