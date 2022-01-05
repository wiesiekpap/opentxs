// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <utility>

#include "1_Internal.hpp"
#include "core/ui/base/List.hpp"
#include "core/ui/base/Widget.hpp"
#include "internal/core/ui/UI.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#endif  // OT_BLOCKCHAIN
#include "opentxs/core/ui/UnitList.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class Identifier;
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using UnitListList = List<
    UnitListExternalInterface,
    UnitListInternalInterface,
    UnitListRowID,
    UnitListRowInterface,
    UnitListRowInternal,
    UnitListRowBlank,
    UnitListSortKey,
    UnitListPrimaryID>;

class UnitList final : public UnitListList
{
public:
    UnitList(
        const api::session::Client& api,
        const identifier::Nym& nymID,
        const SimpleCallback& cb) noexcept;

    ~UnitList() final;

private:
#if OT_BLOCKCHAIN
    OTZMQListenCallback blockchain_balance_cb_;
    OTZMQDealerSocket blockchain_balance_;
#endif  // OT_BLOCKCHAIN
    const ListenerDefinitions listeners_;

    auto construct_row(
        const UnitListRowID& id,
        const UnitListSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto process_account(const Message& message) noexcept -> void;
    auto process_account(const Identifier& id) noexcept -> void;
#if OT_BLOCKCHAIN
    auto process_blockchain_balance(const Message& message) noexcept -> void;
#endif  // OT_BLOCKCHAIN
    auto process_unit(const UnitListRowID& id) noexcept -> void;
#if OT_BLOCKCHAIN
    auto setup_listeners(const ListenerDefinitions& definitions) noexcept
        -> void final;
#endif  // OT_BLOCKCHAIN
    auto startup() noexcept -> void;

    UnitList() = delete;
    UnitList(const UnitList&) = delete;
    UnitList(UnitList&&) = delete;
    auto operator=(const UnitList&) -> UnitList& = delete;
    auto operator=(UnitList&&) -> UnitList& = delete;
};
}  // namespace opentxs::ui::implementation
