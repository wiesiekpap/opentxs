// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "blockchain/bitcoin/p2p/Message.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace p2p
{
namespace bitcoin
{
class Header;
}  // namespace bitcoin
}  // namespace p2p
}  // namespace blockchain
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::message::implementation
{
class Blocktxn final : public internal::Blocktxn, public implementation::Message
{
public:
    auto BlockTransactions() const noexcept -> OTData final { return payload_; }

    Blocktxn(
        const api::Session& api,
        const blockchain::Type network,
        const Data& raw_Blocktxn) noexcept;
    Blocktxn(
        const api::Session& api,
        std::unique_ptr<Header> header,
        const Data& raw_Blocktxn) noexcept;
    Blocktxn(const Blocktxn&) = delete;
    Blocktxn(Blocktxn&&) = delete;
    auto operator=(const Blocktxn&) -> Blocktxn& = delete;
    auto operator=(Blocktxn&&) -> Blocktxn& = delete;

    ~Blocktxn() final = default;

private:
    const OTData payload_;

    using implementation::Message::payload;
    auto payload(AllocateOutput out) const noexcept -> bool final;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::message::implementation
