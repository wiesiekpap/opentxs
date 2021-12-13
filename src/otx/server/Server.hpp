// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <future>
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/AddressType.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "otx/server/MainFile.hpp"
#include "otx/server/Notary.hpp"
#include "otx/server/Transactor.hpp"
#include "otx/server/UserCommandProcessor.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Notary;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
class Nym;
}  // namespace identity

class Data;
class OTPassword;
class PasswordPrompt;
}  // namespace opentxs

namespace opentxs::server
{
class Server
{
public:
    auto API() const -> const api::session::Notary& { return manager_; }
    auto GetConnectInfo(
        core::AddressType& type,
        std::string& hostname,
        std::uint32_t& port) const -> bool;
    auto GetServerID() const noexcept -> const identifier::Server&;
    auto GetServerNym() const -> const identity::Nym&;
    auto TransportKey(Data& pubkey) const -> OTSecret;
    auto IsFlaggedForShutdown() const -> bool;

    void ActivateCron();
    auto CommandProcessor() -> UserCommandProcessor&
    {
        return userCommandProcessor_;
    }
    auto ComputeTimeout() -> std::chrono::milliseconds
    {
        return m_Cron->computeTimeout();
    }
    auto Cron() -> OTCron& { return *m_Cron; }
    auto DropMessageToNymbox(
        const identifier::Server& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message& msg) -> bool;
    auto GetMainFile() -> MainFile& { return mainFile_; }
    auto GetNotary() -> Notary& { return notary_; }
    auto GetTransactor() -> Transactor& { return transactor_; }
    void Init(bool readOnly = false);
    auto LoadServerNym(const identifier::Nym& nymID) -> bool;
    void ProcessCron();
    auto SendInstrumentToNym(
        const identifier::Server& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const OTPayment& payment,
        const char* command) -> bool;
    auto WalletFilename() -> String& { return m_strWalletFilename; }

    Server(
        const opentxs::api::session::Notary& manager,
        const PasswordPrompt& reason);

    ~Server();

private:
    friend MainFile;

    const std::string DEFAULT_EXTERNAL_IP = "127.0.0.1";
    const std::string DEFAULT_BIND_IP = "127.0.0.1";
    const std::string DEFAULT_NAME = "localhost";
    const std::uint32_t DEFAULT_PORT = 7085;
    const std::uint32_t MIN_TCP_PORT = 1024;
    const std::uint32_t MAX_TCP_PORT = 63356;

    const api::session::Notary& manager_;
    const PasswordPrompt& reason_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;
    std::atomic<bool> have_id_;
    MainFile mainFile_;
    Notary notary_;
    Transactor transactor_;
    UserCommandProcessor userCommandProcessor_;
    OTString m_strWalletFilename;
    // Used at least for whether or not to write to the PID.
    bool m_bReadOnly{false};
    // If the server wants to be shut down, it can set
    // this flag so the caller knows to do so.
    bool m_bShutdownFlag{false};
    // A hash of the server contract
    OTServerID m_notaryID;
    // A hash of the public key that signed the server contract
    std::string m_strServerNymID;
    // This is the server's own contract, containing its public key and
    // connect info.
    Nym_p m_nymServer;
    std::unique_ptr<OTCron> m_Cron;  // This is where re-occurring and expiring
                                     // tasks go.
    OTZMQPushSocket notification_socket_;

    auto nymbox_push(const identifier::Nym& nymID, const OTTransaction& item)
        const -> network::zeromq::Message;

    void CreateMainFile(bool& mainFileExists);
    // Note: SendInstrumentToNym and SendMessageToNym CALL THIS.
    // They are higher-level, this is lower-level.
    auto DropMessageToNymbox(
        const identifier::Server& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        transactionType transactionType,
        const Message* msg = nullptr,
        const String& messageString = String::Factory(),
        const char* command = nullptr) -> bool;
    auto parse_seed_backup(const std::string& input) const
        -> std::pair<std::string, std::string>;
    auto ServerNymID() const -> const std::string& { return m_strServerNymID; }
    auto SetNotaryID(const identifier::Server& notaryID) noexcept -> void;
    void SetServerNymID(const char* strNymID) { m_strServerNymID = strNymID; }

    auto SendInstrumentToNym(
        const identifier::Server& notaryID,
        const identifier::Nym& senderNymID,
        const identifier::Nym& recipientNymID,
        const Message& msg) -> bool;

    Server() = delete;
    Server(const Server&) = delete;
    Server(Server&&) = delete;
    auto operator=(const Server&) -> Server& = delete;
    auto operator=(Server&&) -> Server& = delete;
};
}  // namespace opentxs::server
