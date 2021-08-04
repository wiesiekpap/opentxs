// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <iosfwd>
#include <memory>
#include <string>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
namespace api
{
class Core;
}  // namespace api
}  // namespace opentxs

namespace ot = opentxs;

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Message : virtual public bitcoin::Message
{
public:
    auto Encode() const -> OTData final;

    auto header() const -> const Header& final { return *header_; }

    ~Message() override = default;

protected:
    const ot::api::Core& api_;
    std::unique_ptr<Header> header_;

    auto verify_checksum() const noexcept(false) -> void;

    auto init_hash() noexcept -> void;

    Message(
        const api::Core& api,
        const blockchain::Type network,
        const bitcoin::Command command) noexcept;
    Message(const api::Core& api, std::unique_ptr<Header> header) noexcept;

private:
    auto calculate_checksum(const Data& payload) const noexcept -> OTData;

    Message() = delete;
    Message(const Message&) = delete;
    Message(Message&&) = delete;
    auto operator=(const Message&) -> Message& = delete;
    auto operator=(Message&&) -> Message& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
