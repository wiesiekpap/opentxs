// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/network/blockchain/sync/Base.hpp"

namespace opentxs::network::blockchain::sync
{
class OPENTXS_EXPORT Query final : public Base
{
public:
    class Imp;

    OPENTXS_NO_EXPORT Query(Imp* imp) noexcept;

    ~Query() final;

private:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow-field"
    Imp* imp_;
#pragma GCC diagnostic pop

    Query(const Query&) = delete;
    Query(Query&&) = delete;
    auto operator=(const Query&) -> Query& = delete;
    auto operator=(Query&&) -> Query& = delete;
};
}  // namespace opentxs::network::blockchain::sync
