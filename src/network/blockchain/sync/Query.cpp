// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                               // IWYU pragma: associated
#include "1_Internal.hpp"                             // IWYU pragma: associated
#include "opentxs/network/blockchain/sync/Query.hpp"  // IWYU pragma: associated

#include <memory>

#include "network/blockchain/sync/Base.hpp"
#include "opentxs/network/blockchain/sync/Block.hpp"
#include "opentxs/network/blockchain/sync/MessageType.hpp"
#include "opentxs/network/blockchain/sync/State.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::blockchain::sync
{
struct QueryImp final : public Base::Imp {
    const Query& parent_;

    auto asQuery() const noexcept -> const Query& final { return parent_; }

    auto serialize(zeromq::Message& out) const noexcept -> bool final
    {
        return serialize_type(out);
    }

    QueryImp(const Query& parent) noexcept
        : Imp(Imp::default_version_, MessageType::query, {}, {}, {})
        , parent_(parent)
    {
    }

private:
    QueryImp() noexcept;
    QueryImp(const QueryImp&) = delete;
    QueryImp(QueryImp&&) = delete;
    auto operator=(const QueryImp&) -> QueryImp& = delete;
    auto operator=(QueryImp&&) -> QueryImp& = delete;
};

Query::Query(int) noexcept
    : Base(std::make_unique<QueryImp>(*this))
{
}

Query::Query() noexcept
    : Base(std::make_unique<Imp>())
{
}

Query::~Query() = default;
}  // namespace opentxs::network::blockchain::sync
