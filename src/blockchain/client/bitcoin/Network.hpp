// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string>

#include "blockchain/client/Network.hpp"
#include "opentxs/Bytes.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"

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

namespace blockchain
{
namespace block
{
class Header;
}  // namespace block

namespace client
{
namespace internal
{
struct Config;
}  // namespace internal
}  // namespace client
}  // namespace blockchain
}  // namespace opentxs

namespace opentxs::blockchain::client::bitcoin::implementation
{
class Network final : public client::implementation::Network
{
public:
    auto instantiate_header(const ReadView payload) const noexcept
        -> std::unique_ptr<block::Header> final;

    Network(
        const api::Core& api,
        const api::client::internal::Blockchain& blockchain,
        const Type type,
        const internal::Config& config,
        const std::string& seednode,
        const std::string& syncEndpoint);
    ~Network() final;

private:
    using ot_super = client::implementation::Network;

    Network() = delete;
    Network(const Network&) = delete;
    Network(Network&&) = delete;
    auto operator=(const Network&) -> Network& = delete;
    auto operator=(Network&&) -> Network& = delete;
};
}  // namespace opentxs::blockchain::client::bitcoin::implementation
