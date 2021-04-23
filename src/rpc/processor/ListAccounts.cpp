// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "rpc/RPC.tpp"     // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "internal/api/client/Client.hpp"
#include "opentxs/Pimpl.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/storage/Storage.hpp"
#include "opentxs/rpc/ResponseCode.hpp"
#include "opentxs/rpc/request/Base.hpp"
#include "opentxs/rpc/request/ListAccounts.hpp"
#include "opentxs/rpc/response/Base.hpp"
#include "opentxs/rpc/response/ListAccounts.hpp"
#include "rpc/RPC.hpp"

namespace opentxs::rpc::implementation
{
auto RPC::list_accounts(const request::Base& base) const noexcept
    -> response::Base
{
    const auto& in = base.asListAccounts();
    auto ids = response::Base::Identifiers{};
    const auto reply = [&](const auto code) {
        return response::ListAccounts{in, {{0, code}}, std::move(ids)};
    };

    try {
        const auto& session = client_session(base);
        const auto haveNym = (false == in.FilterNym().empty());
        const auto haveServer = (false == in.FilterNotary().empty());
        const auto haveUnit = (false == in.FilterUnit().empty());
        const auto nymOnly = haveNym && (!haveServer) && (!haveUnit);
        const auto serverOnly = (!haveNym) && haveServer && (!haveUnit);
        const auto unitOnly = (!haveNym) && (!haveServer) && haveUnit;
        const auto nymAndServer = haveNym && haveServer && (!haveUnit);
        const auto nymAndUnit = haveNym && (!haveServer) && haveUnit;
        const auto serverAndUnit = (!haveNym) && haveServer && haveUnit;
        const auto all = haveNym && haveServer && haveUnit;
        const auto byNym = [&] {
            auto out = std::set<std::string>{};
            const auto id = session.Factory().NymID(in.FilterNym());
            const auto ids = session.Storage().AccountsByOwner(id);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byServer = [&] {
            auto out = std::set<std::string>{};
            const auto id = session.Factory().ServerID(in.FilterNotary());
            const auto ids = session.Storage().AccountsByServer(id);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };
        const auto byUnit = [&] {
            auto out = std::set<std::string>{};
            const auto id = session.Factory().UnitID(in.FilterNotary());
            const auto ids = session.Storage().AccountsByContract(id);
            std::transform(
                ids.begin(),
                ids.end(),
                std::inserter(out, out.end()),
                [](const auto& item) { return item->str(); });

            return out;
        };

        if (all) {
            const auto nym = byNym();
            const auto server = byServer();
            const auto unit = byUnit();
            auto temp = std::set<std::string>{};
            std::set_intersection(
                nym.begin(),
                nym.end(),
                server.begin(),
                server.end(),
                std::inserter(temp, temp.end()));
            std::set_intersection(
                temp.begin(),
                temp.end(),
                unit.begin(),
                unit.end(),
                std::back_inserter(ids));
        } else if (nymAndServer) {
            const auto nym = byNym();
            const auto server = byServer();
            std::set_intersection(
                nym.begin(),
                nym.end(),
                server.begin(),
                server.end(),
                std::back_inserter(ids));
        } else if (nymAndUnit) {
            const auto nym = byNym();
            const auto unit = byUnit();
            std::set_intersection(
                nym.begin(),
                nym.end(),
                unit.begin(),
                unit.end(),
                std::back_inserter(ids));
        } else if (serverAndUnit) {
            const auto server = byServer();
            const auto unit = byUnit();
            std::set_intersection(
                server.begin(),
                server.end(),
                unit.begin(),
                unit.end(),
                std::back_inserter(ids));
        } else if (nymOnly) {
            auto data = byNym();
            std::move(data.begin(), data.end(), std::back_inserter(ids));
        } else if (serverOnly) {
            auto data = byServer();
            std::move(data.begin(), data.end(), std::back_inserter(ids));
        } else if (unitOnly) {
            auto data = byUnit();
            std::move(data.begin(), data.end(), std::back_inserter(ids));
        } else {
            const auto data = session.Storage().AccountList();
            std::transform(
                data.begin(),
                data.end(),
                std::back_inserter(ids),
                [](const auto& item) { return item.first; });
        }

        return reply(status(ids));
    } catch (...) {

        return reply(ResponseCode::bad_session);
    }
}
}  // namespace opentxs::rpc::implementation
