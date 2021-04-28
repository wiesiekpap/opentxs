// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include "api/client/ui/UpdateManager.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/contact/ContactItemType.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountSummary.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/Blockchains.hpp"
#include "opentxs/ui/Contact.hpp"
#include "opentxs/ui/ContactList.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/MessagableList.hpp"
#include "opentxs/ui/PayableList.hpp"
#include "opentxs/ui/Profile.hpp"
#include "opentxs/ui/Types.hpp"
#include "opentxs/ui/UnitList.hpp"

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
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network

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
class BlockchainSelection;
class BlockchainSelectionQt;
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

namespace implementation
{
class AccountActivity;
class AccountList;
class AccountSummary;
class ActivitySummary;
class ActivityThread;
class BlockchainSelection;
class Contact;
class ContactList;
class MessagableList;
class PayableList;
class Profile;
class UnitList;
}  // namespace implementation
}  // namespace ui

class Flag;
}  // namespace opentxs

namespace opentxs::api::client
{
struct UI::Imp : public Lockable {
public:
    auto AccountActivity(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::AccountActivity&;
    virtual auto AccountActivityQt(
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountActivityQt*
    {
        return nullptr;
    }
    auto AccountList(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::AccountList&;
    virtual auto AccountListQt(
        const identifier::Nym& nym,
        const SimpleCallback cb) const noexcept -> opentxs::ui::AccountListQt*
    {
        return nullptr;
    }
    auto AccountSummary(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::AccountSummary&;
    virtual auto AccountSummaryQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::AccountSummaryQt*
    {
        return nullptr;
    }
    auto ActivateUICallback(const Identifier& widget) const noexcept -> void;
    auto ActivitySummary(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::ActivitySummary&;
    virtual auto ActivitySummaryQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivitySummaryQt*
    {
        return nullptr;
    }
    auto ActivityThread(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::ActivityThread&;
    virtual auto ActivityThreadQt(
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::ActivityThreadQt*
    {
        return nullptr;
    }
    virtual auto BlankModel(const std::size_t columns) const noexcept
        -> QAbstractItemModel*
    {
        return nullptr;
    }
    auto BlockchainNotaryID(const opentxs::blockchain::Type chain)
        const noexcept -> const identifier::Server&;
    auto BlockchainSelection(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> const opentxs::ui::BlockchainSelection&;
    virtual auto BlockchainSelectionQt(
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> opentxs::ui::BlockchainSelectionQt*
    {
        return nullptr;
    }
    auto BlockchainUnitID(const opentxs::blockchain::Type chain) const noexcept
        -> const identifier::UnitDefinition&;
    auto ClearUICallbacks(const Identifier& widget) const noexcept -> void;
    auto Contact(const Identifier& contactID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::Contact&;
    virtual auto ContactQt(const Identifier& contactID, const SimpleCallback cb)
        const noexcept -> opentxs::ui::ContactQt*
    {
        return nullptr;
    }
    auto ContactList(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::ContactList&;
    virtual auto ContactListQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept -> opentxs::ui::ContactListQt*
    {
        return nullptr;
    }
    auto MessagableList(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::MessagableList&;
    virtual auto MessagableListQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept
        -> opentxs::ui::MessagableListQt*
    {
        return nullptr;
    }
    auto PayableList(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback cb) const noexcept
        -> const opentxs::ui::PayableList&;
    virtual auto PayableListQt(
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback cb) const noexcept -> opentxs::ui::PayableListQt*
    {
        return nullptr;
    }
    auto Profile(const identifier::Nym& nymID, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::Profile&;
    virtual auto ProfileQt(
        const identifier::Nym& nymID,
        const SimpleCallback cb) const noexcept -> opentxs::ui::ProfileQt*
    {
        return nullptr;
    }
    auto RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
        const noexcept -> void;
    virtual auto SeedValidator(
        const opentxs::crypto::SeedStyle type,
        const opentxs::crypto::Language lang) const noexcept
        -> const opentxs::ui::SeedValidator*
    {
        return nullptr;
    }
    auto UnitList(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> const opentxs::ui::UnitList&;
    virtual auto UnitListQt(const identifier::Nym& nym, const SimpleCallback cb)
        const noexcept -> opentxs::ui::UnitListQt*
    {
        return nullptr;
    }

    auto Init() noexcept -> void {}
    auto Shutdown() noexcept -> void;

    Imp(const api::client::internal::Manager& api,
        const api::client::internal::Blockchain& blockchain,
        const Flag& running) noexcept;

    ~Imp() override;

protected:
    /** NymID, AccountID */
    using AccountActivityKey = std::pair<OTNymID, OTIdentifier>;
    using AccountListKey = OTNymID;
    /** NymID, currency*/
    using AccountSummaryKey = std::pair<OTNymID, contact::ContactItemType>;
    using ActivitySummaryKey = OTNymID;
    using ActivityThreadKey = std::pair<OTNymID, OTIdentifier>;
    using ContactKey = OTIdentifier;
    using ContactListKey = OTNymID;
    using MessagableListKey = OTNymID;
    /** NymID, currency*/
    using PayableListKey = std::pair<OTNymID, contact::ContactItemType>;
    using ProfileKey = OTNymID;
    using UnitListKey = OTNymID;

    using AccountActivityValue =
        std::unique_ptr<opentxs::ui::implementation::AccountActivity>;
    using AccountListValue =
        std::unique_ptr<opentxs::ui::implementation::AccountList>;
    using AccountSummaryValue =
        std::unique_ptr<opentxs::ui::implementation::AccountSummary>;
    using ActivitySummaryValue =
        std::unique_ptr<opentxs::ui::implementation::ActivitySummary>;
    using ActivityThreadValue =
        std::unique_ptr<opentxs::ui::implementation::ActivityThread>;
    using ContactValue = std::unique_ptr<opentxs::ui::implementation::Contact>;
    using ContactListValue =
        std::unique_ptr<opentxs::ui::implementation::ContactList>;
    using MessagableListValue =
        std::unique_ptr<opentxs::ui::implementation::MessagableList>;
    using PayableListValue =
        std::unique_ptr<opentxs::ui::implementation::PayableList>;
    using ProfileValue = std::unique_ptr<opentxs::ui::implementation::Profile>;
    using AccountActivityMap =
        std::map<AccountActivityKey, AccountActivityValue>;
    using AccountListMap = std::map<AccountListKey, AccountListValue>;
    using AccountSummaryMap = std::map<AccountSummaryKey, AccountSummaryValue>;
    using ActivitySummaryMap =
        std::map<ActivitySummaryKey, ActivitySummaryValue>;
    using ActivityThreadMap = std::map<ActivityThreadKey, ActivityThreadValue>;
    using ContactMap = std::map<ContactKey, ContactValue>;
    using ContactListMap = std::map<ContactListKey, ContactListValue>;
    using MessagableListMap = std::map<MessagableListKey, MessagableListValue>;
    using PayableListMap = std::map<PayableListKey, PayableListValue>;
    using ProfileMap = std::map<ProfileKey, ProfileValue>;
    using UnitListValue =
        std::unique_ptr<opentxs::ui::implementation::UnitList>;
    using UnitListMap = std::map<UnitListKey, UnitListValue>;
    using BlockchainSelectionPointer =
        std::unique_ptr<opentxs::ui::implementation::BlockchainSelection>;
    using BlockchainSelectionMap =
        std::map<opentxs::ui::Blockchains, BlockchainSelectionPointer>;

    const api::client::internal::Manager& api_;
    const api::client::internal::Blockchain& blockchain_;
    const Flag& running_;
    mutable AccountActivityMap accounts_;
    mutable AccountListMap account_lists_;
    mutable AccountSummaryMap account_summaries_;
    mutable ActivitySummaryMap activity_summaries_;
    mutable ContactMap contacts_;
    mutable ContactListMap contact_lists_;
    mutable MessagableListMap messagable_lists_;
    mutable PayableListMap payable_lists_;
    mutable ActivityThreadMap activity_threads_;
    mutable ProfileMap profiles_;
    mutable UnitListMap unit_lists_;
    mutable BlockchainSelectionMap blockchain_selection_;
    ui::UpdateManager update_manager_;

    auto account_activity(
        const Lock& lock,
        const identifier::Nym& nymID,
        const Identifier& accountID,
        const SimpleCallback& cb) const noexcept
        -> AccountActivityMap::mapped_type&;
    auto account_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> AccountListMap::mapped_type&;
    auto account_summary(
        const Lock& lock,
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback& cb) const noexcept
        -> AccountSummaryMap::mapped_type&;
    auto activity_summary(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> ActivitySummaryMap::mapped_type&;
    auto activity_thread(
        const Lock& lock,
        const identifier::Nym& nymID,
        const Identifier& threadID,
        const SimpleCallback& cb) const noexcept
        -> ActivityThreadMap::mapped_type&;
    auto blockchain_selection(
        const Lock& lock,
        const opentxs::ui::Blockchains type,
        const SimpleCallback updateCB) const noexcept
        -> BlockchainSelectionMap::mapped_type&;
    auto contact(
        const Lock& lock,
        const Identifier& contactID,
        const SimpleCallback& cb) const noexcept -> ContactMap::mapped_type&;
    auto contact_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> ContactListMap::mapped_type&;
    auto is_blockchain_account(const Identifier& id) const noexcept
        -> std::optional<opentxs::blockchain::Type>;
    auto messagable_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept
        -> MessagableListMap::mapped_type&;
    auto payable_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const contact::ContactItemType currency,
        const SimpleCallback& cb) const noexcept
        -> PayableListMap::mapped_type&;
    auto profile(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept -> ProfileMap::mapped_type&;
    auto unit_list(
        const Lock& lock,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) const noexcept -> UnitListMap::mapped_type&;

    auto ShutdownCallbacks() noexcept -> void;
    virtual auto ShutdownModels() noexcept -> void;

private:
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::api::client
