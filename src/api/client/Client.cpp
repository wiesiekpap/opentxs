// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "internal/api/client/Client.hpp"  // IWYU pragma: associated

#include <map>
#include <type_traits>

#include "internal/blockchain/Params.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/protobuf/ContactEnums.pb.h"

namespace opentxs
{
auto Translate(const blockchain::Type type) noexcept -> proto::ContactItemType
{
    try {
        return blockchain::params::Data::Chains().at(type).proto_;
    } catch (...) {
        return proto::CITEMTYPE_UNKNOWN;
    }
}

auto Translate(const proto::ContactItemType type) noexcept -> blockchain::Type
{
    using Map =
        std::map<opentxs::proto::ContactItemType, opentxs::blockchain::Type>;

    static const auto build = []() -> auto
    {
        auto output = Map{};

        for (const auto& [chain, data] : blockchain::params::Data::Chains()) {
            output.emplace(data.proto_, chain);
        }

        return output;
    };
    static const auto map{build()};

    try {
        return map.at(type);
    } catch (...) {
        return blockchain::Type::Unknown;
    }
}
}  // namespace opentxs
