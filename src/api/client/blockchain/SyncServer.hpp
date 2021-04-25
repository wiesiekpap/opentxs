// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "internal/api/client/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"

namespace opentxs
{
namespace api
{
namespace client
{
namespace internal
{
struct Blockchain;
}  // namespace internal
}  // namespace client

class Core;
}  // namespace api
}  // namespace opentxs

namespace opentxs::api::client::blockchain
{
struct SyncServer {
    using Blockchain = client::internal::Blockchain;
    using Chain = opentxs::blockchain::Type;

    auto Endpoint(const Chain chain) const noexcept -> std::string;

    auto Disable(const Chain chain) noexcept -> void;
    auto Enable(const Chain chain) noexcept -> void;
    auto Start(
        const std::string& sync,
        const std::string& publicSync,
        const std::string& update,
        const std::string& publicUpdate) noexcept -> bool;

    SyncServer(const api::Core& api, Blockchain& parent) noexcept;

    ~SyncServer();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_p_;
    Imp& imp_;
};
}  // namespace opentxs::api::client::blockchain
