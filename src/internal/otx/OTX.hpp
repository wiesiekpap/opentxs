// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/otx/ConsensusType.hpp"
// IWYU pragma: no_include "opentxs/otx/LastReplyStatus.hpp"
// IWYU pragma: no_include "opentxs/otx/OTXPushType.hpp"
// IWYU pragma: no_include "opentxs/otx/ServerReplyType.hpp"
// IWYU pragma: no_include "opentxs/otx/ServerRequestType.hpp"

#pragma once

#include "opentxs/otx/Types.hpp"
#include "opentxs/protobuf/ConsensusEnums.pb.h"
#include "opentxs/protobuf/OTXEnums.pb.h"

namespace opentxs
{
auto translate(otx::ConsensusType in) noexcept -> proto::ConsensusType;
auto translate(otx::LastReplyStatus in) noexcept -> proto::LastReplyStatus;
auto translate(otx::OTXPushType in) noexcept -> proto::OTXPushType;
auto translate(otx::ServerReplyType in) noexcept -> proto::ServerReplyType;
auto translate(otx::ServerRequestType in) noexcept -> proto::ServerRequestType;
auto translate(proto::ConsensusType in) noexcept -> otx::ConsensusType;
auto translate(proto::LastReplyStatus in) noexcept -> otx::LastReplyStatus;
auto translate(proto::OTXPushType in) noexcept -> otx::OTXPushType;
auto translate(proto::ServerReplyType in) noexcept -> otx::ServerReplyType;
auto translate(proto::ServerRequestType in) noexcept -> otx::ServerRequestType;
}  // namespace opentxs
