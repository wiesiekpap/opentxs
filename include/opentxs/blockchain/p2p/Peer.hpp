// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_BLOCKCHAIN_P2P_PEER_HPP
#define OPENTXS_BLOCKCHAIN_P2P_PEER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <future>

namespace opentxs
{
namespace blockchain
{
namespace p2p
{
class OPENTXS_EXPORT Peer
{
public:
    using ConnectionStatus = std::shared_future<bool>;
    using Handshake = std::shared_future<void>;
    using Verify = std::shared_future<void>;
    using Subscribe = std::shared_future<void>;

    virtual auto Connected() const noexcept -> ConnectionStatus = 0;
    virtual auto HandshakeComplete() const noexcept -> Handshake = 0;

    virtual ~Peer() = default;

protected:
    Peer() noexcept = default;

private:
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;
};
}  // namespace p2p
}  // namespace blockchain
}  // namespace opentxs
#endif
