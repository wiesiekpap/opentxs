// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_PROTOBUF_VERIFYPEER_HPP
#define OPENTXS_PROTOBUF_VERIFYPEER_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/protobuf/Basic.hpp"

namespace opentxs
{
namespace proto
{
auto PeerObjectAllowedNym() noexcept -> const VersionMap&;
auto PeerObjectAllowedPeerReply() noexcept -> const VersionMap&;
auto PeerObjectAllowedPeerRequest() noexcept -> const VersionMap&;
auto PeerObjectAllowedPurse() noexcept -> const VersionMap&;
auto PeerReplyAllowedBailment() noexcept -> const VersionMap&;
auto PeerReplyAllowedConnectionInfo() noexcept -> const VersionMap&;
auto PeerReplyAllowedNotice() noexcept -> const VersionMap&;
auto PeerReplyAllowedOutBailment() noexcept -> const VersionMap&;
auto PeerReplyAllowedSignature() noexcept -> const VersionMap&;
auto PeerRequestAllowedBailment() noexcept -> const VersionMap&;
auto PeerRequestAllowedConnectionInfo() noexcept -> const VersionMap&;
auto PeerRequestAllowedFaucet() noexcept -> const VersionMap&;
auto PeerRequestAllowedOutBailment() noexcept -> const VersionMap&;
auto PeerRequestAllowedPendingBailment() noexcept -> const VersionMap&;
auto PeerRequestAllowedSignature() noexcept -> const VersionMap&;
auto PeerRequestAllowedStoreSecret() noexcept -> const VersionMap&;
auto PeerRequestAllowedVerificationOffer() noexcept -> const VersionMap&;
}  // namespace proto
}  // namespace opentxs
#endif  // OPENTXS_PROTOBUF_VERIFYPEER_HPP
