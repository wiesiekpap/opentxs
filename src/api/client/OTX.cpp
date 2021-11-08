// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"        // IWYU pragma: associated
#include "1_Internal.hpp"      // IWYU pragma: associated
#include "api/client/OTX.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <ctime>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Proto.tpp"
#include "core/StateMachine.hpp"
#include "internal/api/client/Factory.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/otx/client/OTPayment.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Shared.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/api/client/PaymentWorkflowState.hpp"
#include "opentxs/api/client/PaymentWorkflowType.hpp"
#include "opentxs/api/client/Workflow.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/client/OT_API.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"  // IWYU pragma: keep
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Editor.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/contract/peer/BailmentNotice.hpp"
#include "opentxs/core/contract/peer/BailmentReply.hpp"
#include "opentxs/core/contract/peer/BailmentRequest.hpp"
#include "opentxs/core/contract/peer/ConnectionInfoType.hpp"
#include "opentxs/core/contract/peer/ConnectionReply.hpp"
#include "opentxs/core/contract/peer/ConnectionRequest.hpp"
#include "opentxs/core/contract/peer/NoticeAcknowledgement.hpp"
#include "opentxs/core/contract/peer/OutBailmentReply.hpp"
#include "opentxs/core/contract/peer/OutBailmentRequest.hpp"
#include "opentxs/core/contract/peer/StoreSecret.hpp"
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameSection.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/otx/Reply.hpp"
#include "opentxs/otx/ServerReplyType.hpp"
#include "opentxs/otx/Types.hpp"
#include "opentxs/otx/consensus/Server.hpp"
#include "opentxs/protobuf/PeerRequest.pb.h"
#include "opentxs/protobuf/ServerContract.pb.h"
#include "opentxs/protobuf/ServerReply.pb.h"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/WorkType.hpp"
#include "otx/client/PaymentTasks.hpp"
#include "otx/client/StateMachine.hpp"

#define CHECK_NYM(a)                                                           \
    {                                                                          \
        if (a.empty()) {                                                       \
            LogError()(OT_METHOD)(__func__)(": Invalid ")(#a)(".").Flush();    \
                                                                               \
            return error_task();                                               \
        }                                                                      \
    }

#define CHECK_SERVER(a, b)                                                     \
    {                                                                          \
        CHECK_NYM(a)                                                           \
                                                                               \
        if (b.empty()) {                                                       \
            LogError()(OT_METHOD)(__func__)(": Invalid ")(#b)(".").Flush();    \
                                                                               \
            return error_task();                                               \
        }                                                                      \
    }

#define CHECK_ARGS(a, b, c)                                                    \
    {                                                                          \
        CHECK_SERVER(a, b)                                                     \
                                                                               \
        if (c.empty()) {                                                       \
            LogError()(OT_METHOD)(__func__)(": Invalid ")(#c)(".").Flush();    \
                                                                               \
            return error_task();                                               \
        }                                                                      \
    }

#define SHUTDOWN()                                                             \
    {                                                                          \
        YIELD(50);                                                             \
    }

#define YIELD(a)                                                               \
    {                                                                          \
        if (!running_) { return false; }                                       \
                                                                               \
        Sleep(std::chrono::milliseconds(a));                                   \
    }

#define CONTACT_REFRESH_DAYS 1
#define INTRODUCTION_SERVER_KEY "introduction_server_id"
#define MASTER_SECTION "Master"

#define OT_METHOD "opentxs::api::client::implementation::OTX::"

namespace zmq = opentxs::network::zeromq;

namespace opentxs::factory
{
auto OTX(
    const Flag& running,
    const api::session::Client& client,
    [[maybe_unused]] OTClient& otclient,
    const ContextLockCallback& lockCallback) -> api::client::OTX*
{
    return new api::client::implementation::OTX(running, client, lockCallback);
}
}  // namespace opentxs::factory

namespace opentxs::api::client::implementation
{
const std::string OTX::DEFAULT_INTRODUCTION_SERVER =
    R"(-----BEGIN OT ARMORED SERVER CONTRACT-----
Version: Open Transactions 1.13.0
Comment: http://opentransactions.org

CAISJG90Mjl3UFNFdHFyV2loVUM1YVRGRVp3Z3BxZzJicnE0ZGpXdxokb3QyMkoydkFjcFZO
Q3dFUTJHdzE4SHBDa1NQeDlmUzZZcmJHIiJPcGVuLVRyYW5zYWN0aW9ucyBEaXNjb3Zlcnkg
U2VydmVyKvAMCAYSJG90MjJKMnZBY3BWTkN3RVEyR3cxOEhwQ2tTUHg5ZlM2WXJiRxgCKAIy
UwgCEAIiTQgDEiECECNsSN/k/vUJbAgvYE9/vCcMA4Sx6dWXKYEo85D+pOsaIDaSlqQMflBN
JazcCoj83zEDs3Bdh3EIpVnu1xIvJg8igAEAiAEAOuwLCAYSJG90MjJKMnZBY3BWTkN3RVEy
R3cxOEhwQ2tTUHg5ZlM2WXJiRxokb3QyOFg1UzVrQTY0ZlFab05lQmRzTVNGcHlEd0Z2R2dW
MkpuIAIyswQIBhIkb3QyOFg1UzVrQTY0ZlFab05lQmRzTVNGcHlEd0Z2R2dWMkpuGAIgASgC
MiRvdDIySjJ2QWNwVk5Dd0VRMkd3MThIcENrU1B4OWZTNllyYkdCXQgCElMIAhACIk0IAxIh
AhAjbEjf5P71CWwIL2BPf7wnDAOEsenVlymBKPOQ/qTrGiA2kpakDH5QTSWs3AqI/N8xA7Nw
XYdxCKVZ7tcSLyYPIoABAIgBABoECAEQAkqdAQgCEAIaMQgCEAMYAiABKiEDvgNLXNFyUhkO
kIoReOSL9PYbBTcUwTqKUsffuP2d1JZI9rKYwg8aMQgCEAMYAiACKiEDeM7hnv1M69Fi/WWT
3TIPOXuCKWKqH+q2iYauqsdbDcFI9rKYwg8aMQgCEAMYAiADKiEDKqADpj4025+EEI8nVFJB
j5zIDU4iIUteZSJOO9mg8PVI9rKYwg96bggBEiRvdDI4WDVTNWtBNjRmUVpvTmVCZHNNU0Zw
eUR3RnZHZ1YySm4YASAFKkA/H0Lr1GGW/1uzoEq3pkxerFlb4H/Laz87aQfwZ5meIoPULNTs
vk9m5yhAYPXd18gzo0Lg5LiVwQII2b7eIg12em4IARIkb3QyMkoydkFjcFZOQ3dFUTJHdzE4
SHBDa1NQeDlmUzZZcmJHGAMgBSpAfH4+MZvctcpPDVAyst17q1P7GxHzAZ5sRwEKnxvRaB2v
O9Ye1r/iMldIgk3YggQvXNqTVmMdzGKAW1kntf7VCUL8AwgGEiNvdG9wcmlmVG1HdGd2UHFX
RG9NM1d6THpxRnJ5VHIyRnVWQxgCIAIoAjIkb3QyMkoydkFjcFZOQ3dFUTJHdzE4SHBDa1NQ
eDlmUzZZcmJHOigIARIkb3QyOFg1UzVrQTY0ZlFab05lQmRzTVNGcHlEd0Z2R2dWMkpuSp0B
CAIQAhoxCAIQAxgCIAEqIQO9L6bkuSes2sCDqCiaKnZW/k9E1ghqqEBz3WC0Tz8fIEjqqpyl
CBoxCAIQAxgCIAIqIQMbsZwSi9acmxbdg7N+1Qm2sHaYJZUIMZFHCDIh4orHLUjqqpylCBox
CAIQAxgCIAMqIQI1GAp4MULdVrmuKToXG0CjQdCb/nSMoup+A5iSxsT5FkjqqpylCHptCAES
I290b3ByaWZUbUd0Z3ZQcVdEb00zV3pMenFGcnlUcjJGdVZDGAEgBSpAFohlPVxGmi9q5nrp
6lkoZnFkCqpDAVg4pYje/594ZlDfQbRuGBGLr46GqOjHEYgNetlZIPTcuxHVvXEzsPYcAHpu
CAESJG90MjhYNVM1a0E2NGZRWm9OZUJkc01TRnB5RHdGdkdnVjJKbhgBIAUqQE1hhHCg+zav
zI/DKwjFsHMkG8K84B3UJ5YbnK3s26A6uSTv/nK/HAJ2U8hH41aLihJYcqrqmePuPPYiEr/O
dj9C5AIIBhIkb3QyMTg5d1NoaG1EWnJQM3VGM3QyaFZkSHNiMzJwRUVhc1pDGAIgAygBMiRv
dDIySjJ2QWNwVk5Dd0VRMkd3MThIcENrU1B4OWZTNllyYkc6KAgBEiRvdDI4WDVTNWtBNjRm
UVpvTmVCZHNNU0ZweUR3RnZHZ1YySm5adAgGEjYIBhABGjAIBhgFIiJPcGVuLVRyYW5zYWN0
aW9ucyBEaXNjb3ZlcnkgU2VydmVyKAAwADgBOAISOAgGEAIaMggGGA0iJG90Mjl3UFNFdHFy
V2loVUM1YVRGRVp3Z3BxZzJicnE0ZGpXdygAMAA4ATgCem4IARIkb3QyOFg1UzVrQTY0ZlFa
b05lQmRzTVNGcHlEd0Z2R2dWMkpuGAEgBSpA4+Q8k3XxBXd/oIfcVIjay7asTwXphHR7E0ke
0CFSnzR3+h5o4KwLdySDkczUzyNucDCUPw5zQCLxAEaRPvbUGjIXCAEQARgBIgw1NC4zOS4x
MjkuNDUorTcyHwgBEAIYASIUMjYwNzo1MzAwOjIwMzo0MDJkOjoorTc6fENvbXBsaW1lbnRh
cnkgaG9zdGluZyBvZiBueW1zIGFuZCBvdGhlciBvcGVudHhzIGlkZW50aWZpY2F0aW9uIGNv
bnRyYWN0cy4gTm8gbWVzc2FnaW5nIG9yIGZpbmFuY2lhbCB0cmFuc2FjdGlvbnMgaXMgZW5h
YmxlZC5CIJolw7EGFI7d8/eBhdGvmLm71YsI//6vR1AN0R5M7CAFSm0IARIjb3RvcHJpZlRt
R3RndlBxV0RvTTNXekx6cUZyeVRyMkZ1VkMYBSAFKkAX0OBcNXhfPPL+VjKejhN7OX0PlCxO
8Lj0Aax8pKSF55dtKDA8+FIx7gAj56NKRkoS+LPO4mYZ8NcnjGgM2JYS
-----END OT ARMORED SERVER CONTRACT-----)";

OTX::OTX(
    const Flag& running,
    const api::session::Client& client,
    const ContextLockCallback& lockCallback)
    : lock_callback_(lockCallback)
    , running_(running)
    , client_(client)
    , introduction_server_lock_()
    , nym_fetch_lock_()
    , task_status_lock_()
    , refresh_counter_(0)
    , operations_()
    , server_nym_fetch_()
    , missing_nyms_()
    , outdated_nyms_()
    , missing_servers_()
    , missing_unit_definitions_()
    , introduction_server_id_()
    , task_status_()
    , task_message_id_()
    , account_subscriber_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->process_account(message);
          }))
    , account_subscriber_([&] {
        const auto endpoint = client_.Endpoints().AccountUpdate();
        LogDetail()(OT_METHOD)(__func__)(": Connecting to ")(endpoint).Flush();
        auto out = client_.Network().ZeroMQ().SubscribeSocket(
            account_subscriber_callback_.get());
        const auto start = out->Start(endpoint);

        OT_ASSERT(start);

        return out;
    }())
    , notification_listener_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->process_notification(message);
          }))
    , notification_listener_([&] {
        auto out = client_.Network().ZeroMQ().PullSocket(
            notification_listener_callback_,
            zmq::socket::Socket::Direction::Bind);
        const auto start =
            out->Start(client_.Endpoints().InternalProcessPushNotification());

        OT_ASSERT(start);

        return out;
    }())
    , find_nym_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->find_nym(message);
          }))
    , find_nym_listener_([&] {
        auto out = client_.Network().ZeroMQ().PullSocket(
            find_nym_callback_, zmq::socket::Socket::Direction::Bind);
        const auto start = out->Start(client_.Endpoints().FindNym());

        OT_ASSERT(start);

        return out;
    }())
    , find_server_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->find_server(message);
          }))
    , find_server_listener_([&] {
        auto out = client_.Network().ZeroMQ().PullSocket(
            find_server_callback_, zmq::socket::Socket::Direction::Bind);
        const auto start = out->Start(client_.Endpoints().FindServer());

        OT_ASSERT(start);

        return out;
    }())
    , find_unit_callback_(zmq::ListenCallback::Factory(
          [this](const zmq::Message& message) -> void {
              this->find_unit(message);
          }))
    , find_unit_listener_([&] {
        auto out = client_.Network().ZeroMQ().PullSocket(
            find_unit_callback_, zmq::socket::Socket::Direction::Bind);
        const auto start = out->Start(client_.Endpoints().FindUnitDefinition());

        OT_ASSERT(start);

        return out;
    }())
    , task_finished_([&] {
        auto out = client_.Network().ZeroMQ().PublishSocket();
        const auto start = out->Start(client_.Endpoints().TaskComplete());

        OT_ASSERT(start);

        return out;
    }())
    , messagability_([&] {
        auto out = client_.Network().ZeroMQ().PublishSocket();
        const auto start = out->Start(client_.Endpoints().Messagability());

        OT_ASSERT(start);

        return out;
    }())
    , auto_process_inbox_(Flag::Factory(true))
    , next_task_id_(0)
    , shutdown_(false)
    , shutdown_lock_()
    , reason_(client_.Factory().PasswordPrompt("Refresh OTX data with notary"))
{
    // WARNING: do not access client_.Wallet() during construction
}

auto OTX::AcknowledgeBailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const Identifier& requestID,
    const std::string& instructions,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetNymID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    auto time = std::time_t{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == client_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_METHOD)(__func__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(targetNymID);

    try {
        auto instantiatedRequest =
            client_.Factory().BailmentRequest(recipientNym, serializedRequest);
        auto peerreply = client_.Factory().BailmentReply(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            instructions,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {targetNymID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::AcknowledgeConnection(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& recipientID,
    const Identifier& requestID,
    const bool ack,
    const std::string& url,
    const std::string& login,
    const std::string& password,
    const std::string& key,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, recipientID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    if (ack) {
        if (url.empty()) {
            LogError()(OT_METHOD)(__func__)(": Warning: url is empty.").Flush();
        }

        if (login.empty()) {
            LogError()(OT_METHOD)(__func__)(": Warning: login is empty.")
                .Flush();
        }

        if (password.empty()) {
            LogError()(OT_METHOD)(__func__)(": Warning: password is empty.")
                .Flush();
        }

        if (key.empty()) {
            LogError()(OT_METHOD)(__func__)(": Warning: key is empty.").Flush();
        }
    }

    auto time = std::time_t{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == client_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_METHOD)(__func__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(recipientID);

    try {
        auto instantiatedRequest =
            client_.Factory().BailmentRequest(recipientNym, serializedRequest);
        auto peerreply = client_.Factory().ConnectionReply(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            ack,
            url,
            login,
            password,
            key,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {recipientID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::AcknowledgeNotice(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& recipientID,
    const Identifier& requestID,
    const bool ack,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, recipientID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::time_t time{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == client_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_METHOD)(__func__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(recipientID);

    try {
        auto instantiatedRequest =
            client_.Factory().BailmentRequest(recipientNym, serializedRequest);
        auto peerreply = client_.Factory().ReplyAcknowledgement(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            instantiatedRequest->Type(),
            ack,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {recipientID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::AcknowledgeOutbailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& recipientID,
    const Identifier& requestID,
    const std::string& details,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, recipientID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);
    std::time_t time{0};
    auto serializedRequest = proto::PeerRequest{};
    if (false == client_.Wallet().Internal().PeerRequest(
                     nym->ID(),
                     requestID,
                     StorageBox::INCOMINGPEERREQUEST,
                     time,
                     serializedRequest)) {
        LogError()(OT_METHOD)(__func__)(": Failed to load request.").Flush();

        return error_task();
    }

    auto recipientNym = client_.Wallet().Nym(recipientID);

    try {
        auto instantiatedRequest =
            client_.Factory().BailmentRequest(recipientNym, serializedRequest);
        auto peerreply = client_.Factory().OutbailmentReply(
            nym,
            instantiatedRequest->Initiator(),
            requestID,
            serverID,
            details,
            reason_);

        if (setID) { setID(peerreply->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerReplyTask>(
            {recipientID,
             peerreply.as<contract::peer::Reply>(),
             instantiatedRequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::add_task(const TaskID taskID, const ThreadStatus status) const
    -> OTX::BackgroundTask
{
    Lock lock(task_status_lock_);

    if (0 != task_status_.count(taskID)) { return error_task(); }

    std::promise<Result> promise{};
    [[maybe_unused]] auto [it, added] = task_status_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(taskID),
        std::forward_as_tuple(status, std::move(promise)));

    OT_ASSERT(added);

    return {it->first, it->second.second.get_future()};
}

void OTX::associate_message_id(const Identifier& messageID, const TaskID taskID)
    const
{
    Lock lock(task_status_lock_);
    task_message_id_.emplace(taskID, messageID);
}

auto OTX::can_deposit(
    const OTPayment& payment,
    const identifier::Nym& recipient,
    const Identifier& accountIDHint,
    identifier::Server& depositServer,
    identifier::UnitDefinition& unitID,
    Identifier& depositAccount) const -> Depositability
{
    auto nymID = identifier::Nym::Factory();

    if (false == extract_payment_data(payment, nymID, depositServer, unitID)) {

        return Depositability::INVALID_INSTRUMENT;
    }

    auto output = valid_recipient(payment, nymID, recipient);

    if (Depositability::WRONG_RECIPIENT == output) { return output; }

    const bool registered =
        client_.InternalClient().OTAPI().IsNym_RegisteredAtServer(
            recipient, depositServer);

    if (false == registered) {
        schedule_download_nymbox(recipient, depositServer);
        LogDetail()(OT_METHOD)(__func__)(": Recipient nym ")(
            recipient)(" not registered on "
                       "server ")(depositServer)(".")
            .Flush();

        return Depositability::NOT_REGISTERED;
    }

    output = valid_account(
        payment,
        recipient,
        depositServer,
        unitID,
        accountIDHint,
        depositAccount);

    switch (output) {
        case Depositability::ACCOUNT_NOT_SPECIFIED: {
            LogError()(OT_METHOD)(__func__)(
                ": Multiple valid accounts exist. "
                "This payment can not be automatically deposited.")
                .Flush();
        } break;
        case Depositability::WRONG_ACCOUNT: {
            LogDetail()(OT_METHOD)(__func__)(
                ": The specified account is not valid for this payment.")
                .Flush();
        } break;
        case Depositability::NO_ACCOUNT: {

            LogDetail()(OT_METHOD)(__func__)(": Recipient ")(
                recipient)(" needs an account "
                           "for ")(unitID)(" on "
                                           "serve"
                                           "r"
                                           " ")(depositServer)(".")
                .Flush();
        } break;
        case Depositability::READY: {
            LogDetail()(OT_METHOD)(__func__)(": Payment can be deposited.")
                .Flush();
        } break;
        default: {
            OT_FAIL
        }
    }

    return output;
}

auto OTX::can_message(
    const identifier::Nym& senderNymID,
    const Identifier& recipientContactID,
    identifier::Nym& recipientNymID,
    identifier::Server& serverID) const -> Messagability
{
    const auto publish = [&](auto value) {
        return publish_messagability(senderNymID, recipientContactID, value);
    };
    auto senderNym = client_.Wallet().Nym(senderNymID);

    if (false == bool(senderNym)) {
        LogDetail()(OT_METHOD)(__func__)(": Unable to load sender nym ")(
            senderNymID)
            .Flush();

        return publish(Messagability::MISSING_SENDER);
    }

    const bool canSign = senderNym->HasCapability(NymCapability::SIGN_MESSAGE);

    if (false == canSign) {
        LogDetail()(OT_METHOD)(__func__)(": Sender nym ")(
            senderNymID)(" can not sign messages (no private "
                         "key).")
            .Flush();

        return publish(Messagability::INVALID_SENDER);
    }

    const auto contact = client_.Contacts().Contact(recipientContactID);

    if (false == bool(contact)) {
        LogDetail()(OT_METHOD)(__func__)(": Recipient contact ")(
            recipientContactID)(" does not exist.")
            .Flush();

        return publish(Messagability::MISSING_CONTACT);
    }

    const auto nyms = contact->Nyms();

    if (0 == nyms.size()) {
        LogDetail()(OT_METHOD)(__func__)(": Recipient contact ")(
            recipientContactID)(" does not have a nym.")
            .Flush();

        return publish(Messagability::CONTACT_LACKS_NYM);
    }

    Nym_p recipientNym{nullptr};

    for (const auto& it : nyms) {
        recipientNym = client_.Wallet().Nym(it);

        if (recipientNym) {
            recipientNymID.Assign(it);
            break;
        }
    }

    if (false == bool(recipientNym)) {
        for (const auto& nymID : nyms) {
            outdated_nyms_.Push(next_task_id(), nymID);
        }

        LogDetail()(OT_METHOD)(__func__)(": Recipient contact ")(
            recipientContactID)(" credentials not "
                                "available.")
            .Flush();

        return publish(Messagability::MISSING_RECIPIENT);
    }

    OT_ASSERT(recipientNym)

    const auto& claims = recipientNym->Claims();
    serverID.Assign(claims.PreferredOTServer());

    // TODO maybe some of the other nyms in this contact do specify a server
    if (serverID.empty()) {
        LogDetail()(OT_METHOD)(__func__)(": Recipient contact ")(
            recipientContactID)(", "
                                "nym ")(
            recipientNymID)(": credentials do not specify a server.")
            .Flush();
        outdated_nyms_.Push(next_task_id(), recipientNymID);

        return publish(Messagability::NO_SERVER_CLAIM);
    }

    const bool registered =
        client_.InternalClient().OTAPI().IsNym_RegisteredAtServer(
            senderNymID, serverID);

    if (false == registered) {
        schedule_download_nymbox(senderNymID, serverID);
        LogDetail()(OT_METHOD)(__func__)(": Sender nym ")(
            senderNymID)(" not registered on "
                         "server ")(serverID)
            .Flush();

        return publish(Messagability::UNREGISTERED);
    }

    return publish(Messagability::READY);
}

auto OTX::CanDeposit(
    const identifier::Nym& recipientNymID,
    const OTPayment& payment) const -> Depositability
{
    auto accountHint = Identifier::Factory();

    return CanDeposit(recipientNymID, accountHint, payment);
}

auto OTX::CanDeposit(
    const identifier::Nym& recipientNymID,
    const Identifier& accountIDHint,
    const OTPayment& payment) const -> Depositability
{
    auto serverID = identifier::Server::Factory();
    auto unitID = identifier::UnitDefinition::Factory();
    auto accountID = Identifier::Factory();

    return can_deposit(
        payment, recipientNymID, accountIDHint, serverID, unitID, accountID);
}

auto OTX::CanMessage(
    const identifier::Nym& sender,
    const Identifier& contact,
    const bool startIntroductionServer) const -> Messagability
{
    const auto publish = [&](auto value) {
        return publish_messagability(sender, contact, value);
    };

    if (startIntroductionServer) { start_introduction_server(sender); }

    if (sender.empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid sender.").Flush();

        return publish(Messagability::INVALID_SENDER);
    }

    if (contact.empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid recipient.").Flush();

        return publish(Messagability::MISSING_CONTACT);
    }

    auto nymID = identifier::Nym::Factory();
    auto serverID = identifier::Server::Factory();

    return can_message(sender, contact, nymID, serverID);
}

auto OTX::CheckTransactionNumbers(
    const identifier::Nym& nym,
    const identifier::Server& serverID,
    const std::size_t quantity) const -> bool
{
    auto context = client_.Wallet().ServerContext(nym, serverID);

    if (false == bool(context)) {
        LogError()(OT_METHOD)(__func__)(": Nym is not registered").Flush();

        return false;
    }

    const auto available = context->AvailableNumbers();

    if (quantity <= available) { return true; }

    LogVerbose()(OT_METHOD)(__func__)(": Asking server for more numbers.")
        .Flush();

    try {
        auto& queue = get_operations({nym, serverID});
        const auto output =
            queue.StartTask<otx::client::GetTransactionNumbersTask>({});
        const auto& taskID = output.first;

        if (0 == taskID) { return false; }

        auto status = Status(taskID);

        while (ThreadStatus::RUNNING == status) {
            Sleep(std::chrono::milliseconds(100));
            status = Status(taskID);
        }

        if (ThreadStatus::FINISHED_SUCCESS == status) { return true; }

        return false;
    } catch (...) {

        return false;
    }
}

auto OTX::ContextIdle(
    const identifier::Nym& nym,
    const identifier::Server& server) const -> OTX::Finished
{
    try {
        auto& queue = get_operations({nym, server});

        return queue.Wait();
    } catch (...) {
        std::promise<void> empty{};
        auto output = empty.get_future();
        empty.set_value();

        return std::move(output);
    }
}

auto OTX::DepositCheques(const identifier::Nym& nymID) const -> std::size_t
{
    std::size_t output{0};
    const auto workflows = client_.Workflow().List(
        nymID,
        api::client::PaymentWorkflowType::IncomingCheque,
        api::client::PaymentWorkflowState::Conveyed);

    for (const auto& id : workflows) {
        const auto chequeState =
            client_.Workflow().LoadChequeByWorkflow(nymID, id);
        const auto& [state, cheque] = chequeState;

        if (api::client::PaymentWorkflowState::Conveyed != state) { continue; }

        OT_ASSERT(cheque)

        if (queue_cheque_deposit(nymID, *cheque)) { ++output; }
    }

    return output;
}

auto OTX::DepositCheques(
    const identifier::Nym& nymID,
    const std::set<OTIdentifier>& chequeIDs) const -> std::size_t
{
    std::size_t output{0};

    if (chequeIDs.empty()) { return DepositCheques(nymID); }

    for (const auto& id : chequeIDs) {
        const auto chequeState = client_.Workflow().LoadCheque(nymID, id);
        const auto& [state, cheque] = chequeState;

        if (api::client::PaymentWorkflowState::Conveyed != state) { continue; }

        OT_ASSERT(cheque)

        if (queue_cheque_deposit(nymID, *cheque)) { ++output; }
    }

    return {};
}

auto OTX::DepositPayment(
    const identifier::Nym& recipientNymID,
    const std::shared_ptr<const OTPayment>& payment) const
    -> OTX::BackgroundTask
{
    auto notUsed = Identifier::Factory();

    return DepositPayment(recipientNymID, notUsed, payment);
}

auto OTX::DepositPayment(
    const identifier::Nym& recipientNymID,
    const Identifier& accountIDHint,
    const std::shared_ptr<const OTPayment>& payment) const
    -> OTX::BackgroundTask
{
    OT_ASSERT(payment)

    if (recipientNymID.empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid recipient.").Flush();

        return error_task();
    }

    auto serverID = identifier::Server::Factory();
    auto unitID = identifier::UnitDefinition::Factory();
    auto accountID = Identifier::Factory();
    const auto status = can_deposit(
        *payment, recipientNymID, accountIDHint, serverID, unitID, accountID);

    try {
        switch (status) {
            case Depositability::READY:
            case Depositability::NOT_REGISTERED:
            case Depositability::NO_ACCOUNT: {
                start_introduction_server(recipientNymID);
                auto& queue = get_operations({recipientNymID, serverID});

                return queue.payment_tasks_.Queue({unitID, accountID, payment});
            }
            default: {
                LogError()(OT_METHOD)(__func__)(
                    ": Unable to queue payment for download (")(
                    static_cast<std::int8_t>(status))(")")
                    .Flush();

                return error_task();
            }
        }
    } catch (...) {
        return error_task();
    }
}

void OTX::DisableAutoaccept() const { auto_process_inbox_->Off(); }

#if OT_CASH
auto OTX::DownloadMint(
    const identifier::Nym& nym,
    const identifier::Server& server,
    const identifier::UnitDefinition& unit) const -> OTX::BackgroundTask
{
    CHECK_ARGS(nym, server, unit);

    try {
        start_introduction_server(nym);
        auto& queue = get_operations({nym, server});

        return queue.StartTask<otx::client::DownloadMintTask>({unit, 0});
    } catch (...) {

        return error_task();
    }
}
#endif

auto OTX::DownloadNym(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetNymID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::CheckNymTask>(targetNymID);
    } catch (...) {

        return error_task();
    }
}

auto OTX::DownloadNymbox(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID) const -> OTX::BackgroundTask
{
    return schedule_download_nymbox(localNymID, serverID);
}

auto OTX::DownloadServerContract(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Server& contractID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, contractID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::DownloadContractTask>({contractID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::DownloadUnitDefinition(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& contractID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, contractID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::DownloadUnitDefinitionTask>(
            {contractID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::extract_payment_data(
    const OTPayment& payment,
    identifier::Nym& nymID,
    identifier::Server& serverID,
    identifier::UnitDefinition& unitID) const -> bool
{
    if (false == payment.GetRecipientNymID(nymID)) {
        LogError()(OT_METHOD)(__func__)(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    if (false == payment.GetNotaryID(serverID)) {
        LogError()(OT_METHOD)(__func__)(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    OT_ASSERT(false == serverID.empty())

    if (false == payment.GetInstrumentDefinitionID(unitID)) {
        LogError()(OT_METHOD)(__func__)(
            ": Unable to load recipient nym from instrument.")
            .Flush();

        return false;
    }

    OT_ASSERT(false == unitID.empty())

    return true;
}

auto OTX::error_task() -> OTX::BackgroundTask
{
    BackgroundTask output{0, Future{}};

    return output;
}

void OTX::find_nym(const opentxs::network::zeromq::Message& message) const
{
    const auto body = message.Body();

    if (1 >= body.size()) {
        LogError()(OT_METHOD)(__func__)(": Invalid message").Flush();

        return;
    }

    const auto id = [&] {
        auto output = client_.Factory().NymID();
        output->Assign(body.at(1).Bytes());

        return output;
    }();

    if (id->empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid id").Flush();

        return;
    }

    const auto taskID{next_task_id()};
    missing_nyms_.Push(taskID, id);
    trigger_all();
}

void OTX::find_server(const opentxs::network::zeromq::Message& message) const
{
    const auto body = message.Body();

    if (1 >= body.size()) {
        LogError()(OT_METHOD)(__func__)(": Invalid message").Flush();

        return;
    }

    const auto id = [&] {
        auto output = client_.Factory().ServerID();
        output->Assign(body.at(1).Bytes());

        return output;
    }();

    if (id->empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid id").Flush();

        return;
    }

    try {
        client_.Wallet().Server(id);
    } catch (...) {
        const auto taskID{next_task_id()};
        missing_servers_.Push(taskID, id);
        trigger_all();
    }
}

void OTX::find_unit(const opentxs::network::zeromq::Message& message) const
{
    const auto body = message.Body();

    if (1 >= body.size()) {
        LogError()(OT_METHOD)(__func__)(": Invalid message").Flush();

        return;
    }

    const auto id = [&] {
        auto output = client_.Factory().UnitID();
        output->Assign(body.at(1).Bytes());

        return output;
    }();

    if (id->empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid id").Flush();

        return;
    }

    try {
        client_.Wallet().UnitDefinition(id);

        return;
    } catch (...) {
        const auto taskID{next_task_id()};
        missing_unit_definitions_.Push(taskID, id);
        trigger_all();
    }
}

auto OTX::FindNym(const identifier::Nym& nymID) const -> OTX::BackgroundTask
{
    CHECK_NYM(nymID)

    const auto taskID{next_task_id()};
    auto output = start_task(taskID, missing_nyms_.Push(taskID, nymID));
    trigger_all();

    return output;
}

auto OTX::FindNym(
    const identifier::Nym& nymID,
    const identifier::Server& serverIDHint) const -> OTX::BackgroundTask
{
    CHECK_NYM(nymID)

    auto& serverQueue = get_nym_fetch(serverIDHint);
    const auto taskID{next_task_id()};
    auto output = start_task(taskID, serverQueue.Push(taskID, nymID));

    return output;
}

auto OTX::FindServer(const identifier::Server& serverID) const
    -> OTX::BackgroundTask
{
    CHECK_NYM(serverID)

    const auto taskID{next_task_id()};
    auto output = start_task(taskID, missing_servers_.Push(taskID, serverID));

    return output;
}

auto OTX::FindUnitDefinition(const identifier::UnitDefinition& unit) const
    -> OTX::BackgroundTask
{
    CHECK_NYM(unit)

    const auto taskID{next_task_id()};
    auto output =
        start_task(taskID, missing_unit_definitions_.Push(taskID, unit));

    return output;
}

auto OTX::finish_task(const TaskID taskID, const bool success, Result&& result)
    const -> bool
{
    if (success) {
        update_task(taskID, ThreadStatus::FINISHED_SUCCESS, std::move(result));
    } else {
        update_task(taskID, ThreadStatus::FINISHED_FAILED, std::move(result));
    }

    return success;
}

auto OTX::get_introduction_server(const Lock& lock) const -> OTServerID
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_))

    bool keyFound = false;
    auto serverID = String::Factory();
    const bool config = client_.Config().Check_str(
        String::Factory(MASTER_SECTION),
        String::Factory(INTRODUCTION_SERVER_KEY),
        serverID,
        keyFound);

    if (!config || !keyFound || !serverID->Exists()) {

        return import_default_introduction_server(lock);
    }

    return identifier::Server::Factory(serverID);
}

auto OTX::get_nym_fetch(const identifier::Server& serverID) const
    -> UniqueQueue<OTNymID>&
{
    Lock lock(nym_fetch_lock_);

    return server_nym_fetch_[serverID];
}

auto OTX::get_operations(const ContextID& id) const noexcept(false)
    -> otx::client::implementation::StateMachine&
{
    Lock lock(shutdown_lock_);

    if (shutdown_.load()) { throw; }

    return get_task(id);
}

auto OTX::get_task(const ContextID& id) const
    -> otx::client::implementation::StateMachine&
{
    Lock lock(lock_);
    auto it = operations_.find(id);

    if (operations_.end() == it) {
        auto added = operations_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(id),
            std::forward_as_tuple(
                client_,
                *this,
                running_,
                client_,
                id,
                next_task_id_,
                missing_nyms_,
                outdated_nyms_,
                missing_servers_,
                missing_unit_definitions_,
                reason_));
        it = std::get<0>(added);
    }

    return it->second;
}

auto OTX::import_default_introduction_server(const Lock& lock) const
    -> OTServerID
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_))

    const auto serialized = proto::StringToProto<proto::ServerContract>(
        String::Factory(DEFAULT_INTRODUCTION_SERVER.c_str()));

    return set_introduction_server(
        lock, client_.Wallet().Internal().Server(serialized));
}

auto OTX::InitiateBailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID)
    CHECK_NYM(targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = client_.Factory().BailmentRequest(
            nym, targetNymID, instrumentDefinitionID, serverID, reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::InitiateOutbailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Amount amount,
    const std::string& message,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID)
    CHECK_NYM(targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = client_.Factory().OutbailmentRequest(
            nym,
            targetNymID,
            instrumentDefinitionID,
            serverID,
            amount,
            message,
            reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::InitiateRequestConnection(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const contract::peer::ConnectionInfoType& type,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(localNymID, serverID)
    CHECK_NYM(targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = client_.Factory().ConnectionRequest(
            nym, targetNymID, type, serverID, reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::InitiateStoreSecret(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const contract::peer::SecretType& type,
    const std::string& primary,
    const std::string& secondary,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetNymID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    if (primary.empty()) {
        LogError()(OT_METHOD)(__func__)(": Warning: primary is empty.").Flush();
    }

    if (secondary.empty()) {
        LogError()(OT_METHOD)(__func__)(": Warning: secondary is empty.")
            .Flush();
    }

    try {
        auto peerrequest = client_.Factory().StoreSecret(
            nym, targetNymID, type, primary, secondary, serverID, reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::IntroductionServer() const -> const identifier::Server&
{
    Lock lock(introduction_server_lock_);

    if (false == bool(introduction_server_id_)) {
        load_introduction_server(lock);
    }

    OT_ASSERT(introduction_server_id_)

    return *introduction_server_id_;
}

auto OTX::IssueUnitDefinition(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& unitID,
    const core::UnitType advertise,
    const std::string& label) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, unitID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::IssueUnitDefinitionTask>(
            {unitID, label, advertise});
    } catch (...) {

        return error_task();
    }
}

void OTX::load_introduction_server(const Lock& lock) const
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_))

    introduction_server_id_ =
        std::make_unique<OTServerID>(get_introduction_server(lock));
}

auto OTX::MessageContact(
    const identifier::Nym& senderNymID,
    const Identifier& contactID,
    const std::string& message,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = identifier::Server::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    try {
        auto& queue = get_operations({senderNymID, serverID});

        return queue.StartTask<otx::client::MessageTask>(
            {recipientNymID, message, std::make_shared<SetID>(setID)});
    } catch (...) {

        return error_task();
    }
}

auto OTX::MessageStatus(const TaskID taskID) const
    -> std::pair<ThreadStatus, OTX::MessageID>
{
    std::pair<ThreadStatus, MessageID> output{{}, Identifier::Factory()};
    auto& [threadStatus, messageID] = output;
    Lock lock(task_status_lock_);
    threadStatus = status(lock, taskID);

    if (threadStatus == ThreadStatus::FINISHED_SUCCESS) {
        auto it = task_message_id_.find(taskID);

        if (task_message_id_.end() != it) {
            messageID = it->second;
            task_message_id_.erase(it);
        }
    }

    return output;
}

auto OTX::NotifyBailment(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::Nym& targetNymID,
    const identifier::UnitDefinition& instrumentDefinitionID,
    const Identifier& requestID,
    const std::string& txid,
    const Amount amount,
    const SetID setID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, instrumentDefinitionID)
    CHECK_NYM(targetNymID)
    CHECK_NYM(requestID)

    start_introduction_server(localNymID);
    const auto nym = client_.Wallet().Nym(localNymID);

    try {
        auto peerrequest = client_.Factory().BailmentNotice(
            nym,
            targetNymID,
            instrumentDefinitionID,
            serverID,
            requestID,
            txid,
            amount,
            reason_);

        if (setID) { setID(peerrequest->ID()); }

        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::PeerRequestTask>(
            {targetNymID, peerrequest.as<contract::peer::Request>()});
    } catch (const std::exception& e) {
        LogError()(OT_METHOD)(__func__)(": ")(e.what()).Flush();

        return error_task();
    }
}

auto OTX::PayContact(
    const identifier::Nym& senderNymID,
    const Identifier& contactID,
    std::shared_ptr<const OTPayment> payment) const -> OTX::BackgroundTask
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = identifier::Server::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    try {
        auto& queue = get_operations({senderNymID, serverID});

        return queue.StartTask<otx::client::PaymentTask>(
            {recipientNymID, std::shared_ptr<const OTPayment>(payment)});
    } catch (...) {

        return error_task();
    }
}

#if OT_CASH
auto OTX::PayContactCash(
    const identifier::Nym& senderNymID,
    const Identifier& contactID,
    const Identifier& workflowID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(senderNymID, contactID)

    start_introduction_server(senderNymID);
    auto serverID = identifier::Server::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(senderNymID, contactID, recipientNymID, serverID);

    if (Messagability::READY != canMessage) { return error_task(); }

    OT_ASSERT(false == serverID->empty())
    OT_ASSERT(false == recipientNymID->empty())

    try {
        auto& queue = get_operations({senderNymID, serverID});

        return queue.StartTask<otx::client::PayCashTask>(
            {recipientNymID, workflowID});
    } catch (...) {

        return error_task();
    }
}
#endif  // OT_CASH

auto OTX::ProcessInbox(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& accountID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, accountID)

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::ProcessInboxTask>({accountID});
    } catch (...) {

        return error_task();
    }
}

void OTX::process_account(const zmq::Message& message) const
{
    const auto body = message.Body();

    OT_ASSERT(2 < body.size())

    auto accountID = client_.Factory().Identifier();
    accountID->Assign(body.at(1).Bytes());
    const auto balance = Amount{body.at(2)};
    LogVerbose()(OT_METHOD)(__func__)(": Account ")(accountID->str())(
        " balance: ")(balance.str())
        .Flush();
}

void OTX::process_notification(const zmq::Message& message) const
{
    OT_ASSERT(0 < message.Body().size())

    const auto& frame = message.Body().at(0);
    const auto notification =
        otx::Reply::Factory(client_, proto::Factory<proto::ServerReply>(frame));
    const auto& nymID = notification->Recipient();
    const auto& serverID = notification->Server();

    if (false == valid_context(nymID, serverID)) {
        LogError()(OT_METHOD)(__func__)(
            ": No context available to handle notification.")
            .Flush();

        return;
    }

    auto context = client_.Wallet().Internal().mutable_ServerContext(
        nymID, serverID, reason_);

    switch (notification->Type()) {
        case otx::ServerReplyType::Push: {
            context.get().ProcessNotification(client_, notification, reason_);
        } break;
        default: {
            LogError()(OT_METHOD)(__func__)(
                ": Unsupported server reply type: ")(
                value(notification->Type()))(".")
                .Flush();
        }
    }
}

auto OTX::publish_messagability(
    const identifier::Nym& sender,
    const Identifier& contact,
    Messagability value) const noexcept -> Messagability
{
    auto work =
        client_.Network().ZeroMQ().TaggedMessage(WorkType::OTXMessagability);
    work->AddFrame(sender);
    work->AddFrame(contact);
    work->AddFrame(value);
    messagability_->Send(work);

    return value;
}

auto OTX::publish_server_registration(
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const bool forcePrimary) const -> bool
{
    OT_ASSERT(false == nymID.empty())
    OT_ASSERT(false == serverID.empty())

    auto nym = client_.Wallet().mutable_Nym(nymID, reason_);

    return nym.AddPreferredOTServer(serverID.str(), forcePrimary, reason_);
}

auto OTX::PublishServerContract(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& contractID) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, contractID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});
        // TODO server id type

        return queue.StartTask<otx::client::PublishServerContractTask>(
            {client_.Factory().ServerID(contractID.str()), false});
    } catch (...) {

        return error_task();
    }
}

auto OTX::queue_cheque_deposit(
    const identifier::Nym& nymID,
    const Cheque& cheque) const -> bool
{
    auto payment{client_.Factory().Payment(String::Factory(cheque))};

    OT_ASSERT(false != bool(payment));

    payment->SetTempValuesFromCheque(cheque);

    if (cheque.GetRecipientNymID().empty()) {
        payment->SetTempRecipientNymID(nymID);
    }

    std::shared_ptr<OTPayment> ppayment{payment.release()};
    const auto task = DepositPayment(nymID, ppayment);
    const auto taskID = std::get<0>(task);

    return (0 != taskID);
}

void OTX::Refresh() const
{
    refresh_accounts();
    refresh_contacts();
    ++refresh_counter_;
    trigger_all();
}

auto OTX::RefreshCount() const -> std::uint64_t
{
    return refresh_counter_.load();
}

auto OTX::refresh_accounts() const -> bool
{
    LogVerbose()(OT_METHOD)(__func__)(": Begin").Flush();
    const auto serverList = client_.Wallet().ServerList();
    const auto accounts = client_.Storage().AccountList();

    for (const auto& server : serverList) {
        SHUTDOWN()

        const auto serverID = identifier::Server::Factory(server.first);
        LogDetail()(OT_METHOD)(__func__)(": Considering server ")(serverID)
            .Flush();

        for (const auto& nymID : client_.Wallet().LocalNyms()) {
            SHUTDOWN()
            auto logStr = String::Factory(": Nym ");
            logStr->Concatenate("%s", nymID->str().c_str());
            const bool registered =
                client_.InternalClient().OTAPI().IsNym_RegisteredAtServer(
                    nymID, serverID);

            if (registered) {
                logStr->Concatenate(" %s ", "is");
                try {
                    auto& queue = get_operations({nymID, serverID});
                    queue.StartTask<otx::client::DownloadNymboxTask>({});
                } catch (...) {

                    return false;
                }
            } else {
                logStr->Concatenate(" %s ", "is not");
            }

            logStr->Concatenate("%s", " registered here.");
            LogDetail()(OT_METHOD)(__func__)(logStr).Flush();
        }
    }

    SHUTDOWN()

    for (const auto& it : accounts) {
        SHUTDOWN()
        const auto accountID = Identifier::Factory(it.first);
        const auto nymID = client_.Storage().AccountOwner(accountID);
        const auto serverID = client_.Storage().AccountServer(accountID);
        LogDetail()(OT_METHOD)(__func__)(": Account ")(accountID)(": ")(
            "  * Owned by nym: ")(nymID)("  * "
                                         "On "
                                         "server"
                                         ":"
                                         " ")(serverID)
            .Flush();
        try {
            auto& queue = get_operations({nymID, serverID});

            if (0 == queue.StartTask<otx::client::ProcessInboxTask>({accountID})
                         .first) {

                return false;
            }
        } catch (...) {

            return false;
        }
    }

    LogVerbose()(OT_METHOD)(__func__)(": End").Flush();

    return true;
}

auto OTX::refresh_contacts() const -> bool
{
    for (const auto& it : client_.Contacts().ContactList()) {
        SHUTDOWN()

        const auto& contactID = it.first;
        LogVerbose()(OT_METHOD)(__func__)(": Considering contact: ")(contactID)
            .Flush();
        const auto contact =
            client_.Contacts().Contact(Identifier::Factory(contactID));

        OT_ASSERT(contact);

        const auto now = std::time(nullptr);
        const std::chrono::seconds interval(now - contact->LastUpdated());
        const std::chrono::hours limit(24 * CONTACT_REFRESH_DAYS);
        const auto nymList = contact->Nyms();

        if (nymList.empty()) {
            LogVerbose()(OT_METHOD)(__func__)(
                ": No nyms associated with this contact.")
                .Flush();

            continue;
        }

        for (const auto& nymID : nymList) {
            SHUTDOWN()

            const auto nym = client_.Wallet().Nym(nymID);
            LogVerbose()(OT_METHOD)(__func__)(": Considering nym: ")(nymID)
                .Flush();

            if (nym) {
                client_.Contacts().Update(*nym);
            } else {
                LogVerbose()(OT_METHOD)(__func__)(
                    ": We don't have credentials for this nym. "
                    " Will search on all servers.")
                    .Flush();
                const auto taskID{next_task_id()};
                missing_nyms_.Push(taskID, nymID);

                continue;
            }

            if (interval > limit) {
                LogVerbose()(OT_METHOD)(__func__)(
                    ": Hours since last update "
                    "(")(interval.count())(") exceeds "
                                           "the limit "
                                           "(")(limit.count())(")")
                    .Flush();
                // TODO add a method to Contact that returns the list of
                // servers
                const auto data = contact->Data();

                if (false == bool(data)) { continue; }

                const auto serverGroup = data->Group(
                    contact::SectionType::Communication,
                    contact::ClaimType::Opentxs);

                if (false == bool(serverGroup)) {

                    const auto taskID{next_task_id()};
                    outdated_nyms_.Push(taskID, nymID);
                    continue;
                }

                for (const auto& [claimID, item] : *serverGroup) {
                    SHUTDOWN()
                    OT_ASSERT(item)

                    const auto& notUsed [[maybe_unused]] = claimID;
                    const auto serverID =
                        identifier::Server::Factory(item->Value());

                    if (serverID->empty()) { continue; }

                    LogVerbose()(OT_METHOD)(__func__)(": Will download nym ")(
                        nymID)(" from "
                               "server ")(serverID)
                        .Flush();
                    auto& serverQueue = get_nym_fetch(serverID);
                    const auto taskID{next_task_id()};
                    serverQueue.Push(taskID, nymID);
                }
            } else {
                LogVerbose()(OT_METHOD)(__func__)(
                    ": No need to update this nym.")
                    .Flush();
            }
        }
    }

    return true;
}

auto OTX::RegisterAccount(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& unitID,
    const std::string& label) const -> OTX::BackgroundTask
{
    return schedule_register_account(localNymID, serverID, unitID, label);
}

auto OTX::RegisterNym(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const bool resync) const -> OTX::BackgroundTask
{
    CHECK_SERVER(localNymID, serverID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::RegisterNymTask>({resync});
    } catch (...) {

        return error_task();
    }
}

auto OTX::RegisterNymPublic(
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const bool setContactData,
    const bool forcePrimary,
    const bool resync) const -> OTX::BackgroundTask
{
    CHECK_SERVER(nymID, serverID)

    start_introduction_server(nymID);

    if (setContactData) {
        publish_server_registration(nymID, serverID, forcePrimary);
    }

    return RegisterNym(nymID, serverID, resync);
}

auto OTX::SetIntroductionServer(const contract::Server& contract) const
    -> OTServerID
{
    Lock lock(introduction_server_lock_);

    return set_introduction_server(lock, contract);
}

auto OTX::schedule_download_nymbox(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID) const -> OTX::BackgroundTask
{
    CHECK_SERVER(localNymID, serverID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::DownloadNymboxTask>({});
    } catch (...) {

        return error_task();
    }
}

auto OTX::schedule_register_account(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const identifier::UnitDefinition& unitID,
    const std::string& label) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, unitID)

    try {
        start_introduction_server(localNymID);
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::RegisterAccountTask>(
            {label, unitID});
    } catch (...) {

        return error_task();
    }
}

auto OTX::SendCheque(
    const identifier::Nym& localNymID,
    const Identifier& sourceAccountID,
    const Identifier& recipientContactID,
    const Amount value,
    const std::string& memo,
    const Time validFrom,
    const Time validTo) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, sourceAccountID, recipientContactID)

    start_introduction_server(localNymID);
    auto serverID = identifier::Server::Factory();
    auto recipientNymID = identifier::Nym::Factory();
    const auto canMessage =
        can_message(localNymID, recipientContactID, recipientNymID, serverID);
    const bool closeEnough = (Messagability::READY == canMessage) ||
                             (Messagability::UNREGISTERED == canMessage);

    if (false == closeEnough) {
        LogError()(OT_METHOD)(__func__)(": Unable to message contact.").Flush();

        return error_task();
    }

    if (0 >= value) {
        LogError()(OT_METHOD)(__func__)(": Invalid amount.").Flush();

        return error_task();
    }

    auto account = client_.Wallet().Internal().Account(sourceAccountID);

    if (false == bool(account)) {
        LogError()(OT_METHOD)(__func__)(": Invalid account.").Flush();

        return error_task();
    }

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::SendChequeTask>(
            {sourceAccountID, recipientNymID, value, memo, validFrom, validTo});
    } catch (...) {

        return error_task();
    }
}

auto OTX::SendExternalTransfer(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const Amount& value,
    const std::string& memo) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetAccountID)
    CHECK_NYM(sourceAccountID)

    auto sourceAccount = client_.Wallet().Internal().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        LogError()(OT_METHOD)(__func__)(": Invalid source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetNymID() != localNymID) {
        LogError()(OT_METHOD)(__func__)(": Wrong owner on source account.")
            .Flush();

        return error_task();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        LogError()(OT_METHOD)(__func__)(": Wrong notary on source account.")
            .Flush();

        return error_task();
    }

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::SendTransferTask>(
            {sourceAccountID, targetAccountID, value, memo});
    } catch (...) {

        return error_task();
    }
}

auto OTX::SendTransfer(
    const identifier::Nym& localNymID,
    const identifier::Server& serverID,
    const Identifier& sourceAccountID,
    const Identifier& targetAccountID,
    const Amount& value,
    const std::string& memo) const -> OTX::BackgroundTask
{
    CHECK_ARGS(localNymID, serverID, targetAccountID)
    CHECK_NYM(sourceAccountID)

    auto sourceAccount = client_.Wallet().Internal().Account(sourceAccountID);

    if (false == bool(sourceAccount)) {
        LogError()(OT_METHOD)(__func__)(": Invalid source account.").Flush();

        return error_task();
    }

    if (sourceAccount.get().GetNymID() != localNymID) {
        LogError()(OT_METHOD)(__func__)(": Wrong owner on source account.")
            .Flush();

        return error_task();
    }

    if (sourceAccount.get().GetRealNotaryID() != serverID) {
        LogError()(OT_METHOD)(__func__)(": Wrong notary on source account.")
            .Flush();

        return error_task();
    }

    try {
        auto& queue = get_operations({localNymID, serverID});

        return queue.StartTask<otx::client::SendTransferTask>(
            {sourceAccountID, targetAccountID, value, memo});
    } catch (...) {

        return error_task();
    }
}

void OTX::set_contact(
    const identifier::Nym& nymID,
    const identifier::Server& serverID) const
{
    auto nym = client_.Wallet().mutable_Nym(nymID, reason_);
    const auto server = nym.PreferredOTServer();

    if (server.empty()) {
        nym.AddPreferredOTServer(serverID.str(), true, reason_);
    }
}

auto OTX::set_introduction_server(
    const Lock& lock,
    const contract::Server& contract) const -> OTServerID
{
    OT_ASSERT(CheckLock(lock, introduction_server_lock_));

    try {
        auto serialized = proto::ServerContract{};
        if (false == contract.Serialize(serialized, true)) {
            LogError()(OT_METHOD)(__func__)(
                ": Failed to serialize server contract.")
                .Flush();
        }
        const auto instantiated =
            client_.Wallet().Internal().Server(serialized);
        const auto id = identifier::Server::Factory(
            instantiated->ID()->str());  // TODO conversion
        introduction_server_id_ = std::make_unique<OTServerID>(id);

        OT_ASSERT(introduction_server_id_)

        bool dontCare = false;
        const bool set = client_.Config().Set_str(
            String::Factory(MASTER_SECTION),
            String::Factory(INTRODUCTION_SERVER_KEY),
            String::Factory(id),
            dontCare);

        OT_ASSERT(set)

        client_.Config().Save();

        return id;
    } catch (...) {
        return client_.Factory().ServerID();
    }
}

void OTX::start_introduction_server(const identifier::Nym& nymID) const
{
    auto& serverID = IntroductionServer();

    if (serverID.empty()) { return; }

    try {
        auto& queue = get_operations({nymID, serverID});
        queue.StartTask<otx::client::DownloadNymboxTask>({});
    } catch (...) {

        return;
    }
}

auto OTX::start_task(const TaskID taskID, bool success) const
    -> OTX::BackgroundTask
{
    if (0 == taskID) {
        LogTrace()(OT_METHOD)(__func__)(": Empty task ID").Flush();

        return error_task();
    }

    if (false == success) {
        LogTrace()(OT_METHOD)(__func__)(": Task already queued").Flush();

        return error_task();
    }

    return add_task(taskID, ThreadStatus::RUNNING);
}

void OTX::StartIntroductionServer(const identifier::Nym& localNymID) const
{
    start_introduction_server(localNymID);
}

auto OTX::status(const Lock& lock, const TaskID taskID) const -> ThreadStatus
{
    OT_ASSERT(CheckLock(lock, task_status_lock_))

    if (!running_) { return ThreadStatus::SHUTDOWN; }

    auto it = task_status_.find(taskID);

    if (task_status_.end() == it) { return ThreadStatus::Error; }

    const auto output = it->second.first;
    const bool success = (ThreadStatus::FINISHED_SUCCESS == output);
    const bool failed = (ThreadStatus::FINISHED_FAILED == output);
    const bool finished = (success || failed);

    if (finished) { task_status_.erase(it); }

    return output;
}

auto OTX::Status(const TaskID taskID) const -> ThreadStatus
{
    Lock lock(task_status_lock_);

    return status(lock, taskID);
}

void OTX::trigger_all() const
{
    Lock lock(shutdown_lock_);

    for (const auto& [id, queue] : operations_) {
        if (false == queue.Trigger()) { return; }
    }
}

void OTX::update_task(
    const TaskID taskID,
    const ThreadStatus status,
    Result&& result) const noexcept
{
    if (0 == taskID) { return; }

    Lock lock(task_status_lock_);

    if (0 == task_status_.count(taskID)) { return; }

    try {
        auto& row = task_status_.at(taskID);
        auto& [state, promise] = row;
        state = status;
        bool value{false};
        bool publish{false};

        switch (status) {
            case ThreadStatus::FINISHED_SUCCESS: {
                value = true;
                publish = true;
                promise.set_value(std::move(result));
            } break;
            case ThreadStatus::FINISHED_FAILED: {
                value = false;
                publish = true;
                promise.set_value(std::move(result));
            } break;
            case ThreadStatus::SHUTDOWN: {
                Result cancel{otx::LastReplyStatus::Unknown, nullptr};
                promise.set_value(std::move(cancel));
            } break;
            case ThreadStatus::Error:
            case ThreadStatus::RUNNING:
            default: {
            }
        }

        if (publish) {
            const auto& socket = task_finished_.get();
            auto work =
                socket.Context().TaggedMessage(WorkType::OTXTaskComplete);
            work->AddFrame(taskID);
            work->AddFrame(value);
            socket.Send(work);
        }
    } catch (...) {
        LogError()(OT_METHOD)(__func__)(
            ": Tried to finish an already-finished task (")(taskID)(")")
            .Flush();
    }
}

auto OTX::valid_account(
    const OTPayment& payment,
    const identifier::Nym& recipient,
    const identifier::Server& paymentServerID,
    const identifier::UnitDefinition& paymentUnitID,
    const Identifier& accountIDHint,
    Identifier& depositAccount) const -> Depositability
{
    std::set<OTIdentifier> matchingAccounts{};

    for (const auto& it : client_.Storage().AccountList()) {
        const auto accountID = Identifier::Factory(it.first);
        const auto nymID = client_.Storage().AccountOwner(accountID);
        const auto serverID = client_.Storage().AccountServer(accountID);
        const auto unitID = client_.Storage().AccountContract(accountID);

        if (nymID != recipient) { continue; }

        if (serverID != paymentServerID) { continue; }

        if (unitID != paymentUnitID) { continue; }

        matchingAccounts.emplace(accountID);
    }

    if (accountIDHint.empty()) {
        if (0 == matchingAccounts.size()) {

            return Depositability::NO_ACCOUNT;
        } else if (1 == matchingAccounts.size()) {
            depositAccount.Assign(*matchingAccounts.begin());

            return Depositability::READY;
        } else {

            return Depositability::ACCOUNT_NOT_SPECIFIED;
        }
    }

    if (0 == matchingAccounts.size()) {

        return Depositability::NO_ACCOUNT;
    } else if (1 == matchingAccounts.count(accountIDHint)) {
        depositAccount.Assign(accountIDHint);

        return Depositability::READY;
    } else {

        return Depositability::WRONG_ACCOUNT;
    }
}

auto OTX::valid_context(
    const identifier::Nym& nymID,
    const identifier::Server& serverID) const -> bool
{
    const auto nyms = client_.Wallet().LocalNyms();

    if (0 == nyms.count(nymID)) {
        LogError()(OT_METHOD)(__func__)(": Nym ")(
            nymID)(" does not belong to this wallet.")
            .Flush();

        return false;
    }

    if (serverID.empty()) {
        LogError()(OT_METHOD)(__func__)(": Invalid server.").Flush();

        return false;
    }

    const auto context = client_.Wallet().ServerContext(nymID, serverID);

    if (false == bool(context)) {
        LogError()(OT_METHOD)(__func__)(": Context does not exist.").Flush();

        return false;
    }

    if (0 == context->Request()) {
        LogError()(OT_METHOD)(__func__)(
            ": Nym is not registered at this server.")
            .Flush();

        return false;
    }

    return true;
}

auto OTX::valid_recipient(
    const OTPayment& payment,
    const identifier::Nym& specified,
    const identifier::Nym& recipient) const -> Depositability
{
    if (specified.empty()) {
        LogError()(OT_METHOD)(__func__)(": Payment can be accepted by any nym.")
            .Flush();

        return Depositability::READY;
    }

    if (recipient == specified) { return Depositability::READY; }

    return Depositability::WRONG_RECIPIENT;
}

#if OT_CASH
auto OTX::WithdrawCash(
    const identifier::Nym& nymID,
    const identifier::Server& serverID,
    const Identifier& account,
    const Amount amount) const -> OTX::BackgroundTask
{
    CHECK_ARGS(nymID, serverID, account)

    try {
        start_introduction_server(nymID);
        auto& queue = get_operations({nymID, serverID});

        return queue.StartTask<otx::client::WithdrawCashTask>(
            {account, amount});
    } catch (...) {

        return error_task();
    }
}
#endif

OTX::~OTX()
{
    account_subscriber_->Close();
    notification_listener_->Close();
    find_unit_listener_->Close();
    find_server_listener_->Close();
    find_nym_listener_->Close();

    Lock lock(shutdown_lock_);
    shutdown_.store(true);
    lock.unlock();
    std::vector<otx::client::implementation::StateMachine::WaitFuture>
        futures{};

    for (const auto& [id, queue] : operations_) {
        futures.emplace_back(queue.Stop());
    }

    for (const auto& future : futures) { future.get(); }

    for (auto& it : task_status_) {
        auto& promise = it.second.second;

        try {
            promise.set_value(error_result());
        } catch (...) {
        }
    }
}
}  // namespace opentxs::api::client::implementation
