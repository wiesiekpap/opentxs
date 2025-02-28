// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "api/session/base/Scheduler.hpp"  // IWYU pragma: associated

#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Time.hpp"
#include "serialization/protobuf/Nym.pb.h"
#include "serialization/protobuf/ServerContract.pb.h"
#include "serialization/protobuf/UnitDefinition.pb.h"
#include "util/Thread.hpp"

namespace opentxs::api::session
{
Scheduler::Scheduler(const api::Context& parent, Flag& running)
    : Lockable()
    , parent_(parent)
    , running_(running)
    , periodic_()
{
}

void Scheduler::Start(const api::session::Storage* const storage)
{
    OT_ASSERT(nullptr != storage);

    // TODO resolve issues
    /*
    const auto now = std::chrono::seconds(std::time(nullptr));

    Schedule(
        std::chrono::seconds(nym_publish_interval_),
        [=, &dht]() -> void {
            NymLambda nymLambda(
                [=,
                 &dht](const identity::internal::Nym::Serialized& nym) -> void {
                    SetThisThreadsName(schedulerPublishNymThreadName);
                    dht.Internal().Insert(nym);
                });
            storage->MapPublicNyms(nymLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(nym_refresh_interval_),
        [=, &dht]() -> void {
            NymLambda nymLambda([=, &dht](const proto::Nym& nym) -> void {
                SetThisThreadsName(schedulerRefreshNymThreadName);
                dht.GetPublicNym(nym.nymid());
            });
            storage->MapPublicNyms(nymLambda);
        },
        (now - std::chrono::seconds(nym_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(server_publish_interval_),
        [=, &dht]() -> void {
            ServerLambda serverLambda(
                [=, &dht](const proto::ServerContract& server) -> void {
                    SetThisThreadsName(schedulerPublishServerThreadName);
                    dht.Internal().Insert(server);
                });
            storage->MapServers(serverLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(server_refresh_interval_),
        [=, &dht]() -> void {
            ServerLambda serverLambda(
                [=, &dht](const proto::ServerContract& server) -> void {
                    SetThisThreadsName(schedulerRefreshServerThreadName);
                    dht.GetServerContract(server.id());
                });
            storage->MapServers(serverLambda);
        },
        (now - std::chrono::seconds(server_refresh_interval_) / 2));

    Schedule(
        std::chrono::seconds(unit_publish_interval_),
        [=, &dht]() -> void {
            UnitLambda unitLambda(
                [=, &dht](const proto::UnitDefinition& unit) -> void {
                    SetThisThreadsName(schedulerPublishUnitThreadName);
                    dht.Internal().Insert(unit);
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        now);

    Schedule(
        std::chrono::seconds(unit_refresh_interval_),
        [=, &dht]() -> void {
            UnitLambda unitLambda(
                [=, &dht](const proto::UnitDefinition& unit) -> void {
                    SetThisThreadsName(schedulerRefreshUnitThreadName);
                    dht.GetUnitDefinition(unit.id());
                });
            storage->MapUnitDefinitions(unitLambda);
        },
        (now - std::chrono::seconds(unit_refresh_interval_) / 2));
    */
    periodic_ = std::thread(&Scheduler::thread, this);
}

void Scheduler::thread()
{
    SetThisThreadsName(schedulerThreadName);

    while (running_) {
        // Storage has its own interval checking.
        storage_gc_hook();
        std::this_thread::sleep_for(100ms);
    }
}

Scheduler::~Scheduler()
{
    if (periodic_.joinable()) { periodic_.join(); }
}
}  // namespace opentxs::api::session
