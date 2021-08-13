// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "internal/blockchain/node/Node.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <sstream>

#include "opentxs/network/zeromq/socket/Sender.tpp"  // IWYU pragma: keep

namespace opentxs::blockchain::node::internal
{
auto Config::print() const noexcept -> std::string
{
    constexpr auto print_bool = [](const bool in) {
        if (in) {
            return "true";
        } else {
            return "false";
        }
    };

    auto output = std::stringstream{};
    output << "Blockchain client options\n";
    output << "  * download cfilters: " << print_bool(download_cfilters_)
           << '\n';
    output << "  * generate cfilters: " << print_bool(generate_cfilters_)
           << '\n';
    output << "  * provide sync server: " << print_bool(provide_sync_server_)
           << '\n';
    output << "  * use sync server: " << print_bool(use_sync_server_) << '\n';
    output << "  * disable wallet: " << print_bool(disable_wallet_) << '\n';

    return output.str();
}
}  // namespace opentxs::blockchain::node::internal
