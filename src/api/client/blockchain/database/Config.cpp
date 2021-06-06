// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "api/client/blockchain/database/Config.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}

#include <utility>

#include "api/client/blockchain/database/Database.hpp"
#include "internal/api/client/blockchain/Blockchain.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Endpoints.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/LMDB.hpp"

// #define OT_METHOD
// "opentxs::api::client::blockchain::database::implementation::Config::"

namespace opentxs::api::client::blockchain::database::implementation
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

Config::Config(
    const api::Core& api,
    opentxs::storage::lmdb::LMDB& lmdb) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , config_table_(Table::ConfigMulti)
    , socket_([&] {
        auto out = api_.ZeroMQ().PublishSocket();
        const auto rc =
            out->Start(api_.Endpoints().BlockchainSyncServerUpdated());

        OT_ASSERT(rc);

        return out;
    }())
{
}

auto Config::AddSyncServer(const std::string& endpoint) const noexcept -> bool
{
    if (endpoint.empty()) { return false; }

    const auto [success, code] = lmdb_.Store(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        endpoint,
        nullptr,
        MDB_NODUPDATA);

    if (success) {
        auto work = api_.ZeroMQ().TaggedMessage(WorkType::SyncServerUpdated);
        work->AddFrame(endpoint);
        work->AddFrame(success);
        socket_->Send(work);

        return true;
    }

    return MDB_KEYEXIST == code;
}

auto Config::DeleteSyncServer(const std::string& endpoint) const noexcept
    -> bool
{
    if (endpoint.empty()) { return false; }

    const auto output = lmdb_.Delete(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        endpoint,
        nullptr);

    if (output) {
        static constexpr auto deleted{false};
        auto work = api_.ZeroMQ().TaggedMessage(WorkType::SyncServerUpdated);
        work->AddFrame(endpoint);
        work->AddFrame(deleted);
        socket_->Send(work);

        return true;
    }

    return true;
}

auto Config::GetSyncServers() const noexcept -> Endpoints
{
    auto output = Endpoints{};
    lmdb_.Load(
        config_table_,
        tsv(Database::Key::SyncServerEndpoint),
        [&](const auto view) { output.emplace_back(view); },
        opentxs::storage::lmdb::LMDB::Mode::Multiple);

    return output;
}
}  // namespace opentxs::api::client::blockchain::database::implementation
