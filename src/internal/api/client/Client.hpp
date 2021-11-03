// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/api/client/PaymentWorkflowState.hpp"
// IWYU pragma: no_include "opentxs/api/client/PaymentWorkflowType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/crypto/SubaccountType.hpp"
// IWYU pragma: no_include "opentxs/contact/ClaimType.hpp"

#pragma once

#include <functional>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "internal/api/session/Session.hpp"
#include "internal/core/Core.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/client/Activity.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/OTX.hpp"
#include "opentxs/api/client/Pair.hpp"
#include "opentxs/api/client/Types.hpp"
#include "opentxs/api/client/UI.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/UniqueQueue.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/protobuf/PaymentWorkflowEnums.pb.h"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct UI;
}  // namespace internal

class Issuer;
class ServerAction;
class Workflow;
}  // namespace client

namespace crypto
{
class Blockchain;
}  // namespace crypto

namespace session
{
class Wallet;
}  // namespace session

class Crypto;
class Legacy;
class Settings;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block
}  // namespace blockchain

namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace network
{
namespace blockchain
{
namespace sync
{
class Data;
}  // namespace sync
}  // namespace blockchain

namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class Issuer;
}  // namespace proto

class Contact;
class Flag;
class Identifier;
class OTClient;
class OTPayment;
class PasswordPrompt;
template <class T>
class UniqueQueue;
}  // namespace opentxs

namespace opentxs
{
auto translate(const api::client::PaymentWorkflowState in) noexcept
    -> proto::PaymentWorkflowState;
auto translate(const api::client::PaymentWorkflowType in) noexcept
    -> proto::PaymentWorkflowType;
auto translate(const proto::PaymentWorkflowState in) noexcept
    -> api::client::PaymentWorkflowState;
auto translate(const proto::PaymentWorkflowType in) noexcept
    -> api::client::PaymentWorkflowType;
}  // namespace opentxs

namespace opentxs::api::client::internal
{
struct Contacts : virtual public api::client::Contacts {
    virtual auto init(
        const std::shared_ptr<const crypto::Blockchain>& blockchain)
        -> void = 0;
    virtual auto prepare_shutdown() -> void = 0;
    virtual auto start() -> void = 0;

    ~Contacts() override = default;
};
struct OTX : virtual public api::client::OTX {
    virtual void associate_message_id(
        const Identifier& messageID,
        const TaskID taskID) const = 0;
    virtual auto can_deposit(
        const OTPayment& payment,
        const identifier::Nym& recipient,
        const Identifier& accountIDHint,
        identifier::Server& depositServer,
        identifier::UnitDefinition& unitID,
        Identifier& depositAccount) const -> Depositability = 0;
    virtual auto finish_task(
        const TaskID taskID,
        const bool success,
        Result&& result) const -> bool = 0;
    virtual auto get_nym_fetch(const identifier::Server& serverID) const
        -> UniqueQueue<OTNymID>& = 0;
    virtual auto start_task(const TaskID taskID, bool success) const
        -> BackgroundTask = 0;
};
struct Pair : virtual public opentxs::api::client::Pair {
    virtual void init() noexcept = 0;

    ~Pair() override = default;
};
struct UI final : public opentxs::api::client::UI {
    auto ActivateUICallback(const Identifier& widget) const noexcept -> void;
    auto ClearUICallbacks(const Identifier& widget) const noexcept -> void;
    auto RegisterUICallback(const Identifier& widget, const SimpleCallback& cb)
        const noexcept -> void;

    auto Init() noexcept -> void;
    auto Shutdown() noexcept -> void;

    UI(client::UI::Imp*) noexcept;

    ~UI() final = default;

private:
    UI() = delete;
    UI(const UI&) = delete;
    UI(UI&&) = delete;
    UI& operator=(const UI&) = delete;
    UI& operator=(UI&&) = delete;
};
}  // namespace opentxs::api::client::internal
