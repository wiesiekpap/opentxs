// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/rpc/AccountEventType.hpp"
// IWYU pragma: no_include "opentxs/rpc/AccountType.hpp"
// IWYU pragma: no_include "opentxs/rpc/CommandType.hpp"
// IWYU pragma: no_include "opentxs/rpc/ContactEventType.hpp"
// IWYU pragma: no_include "opentxs/rpc/PaymentType.hpp"
// IWYU pragma: no_include "opentxs/rpc/PushType.hpp"
// IWYU pragma: no_include "opentxs/rpc/ResponseCode.hpp"

#pragma once

#include <memory>

#include "opentxs/protobuf/RPCEnums.pb.h"
#include "opentxs/rpc/Types.hpp"

namespace opentxs
{
namespace proto
{
class RPCCommand;
class RPCResponse;
}  // namespace proto

namespace rpc
{
namespace request
{
class Base;
}  // namespace request

namespace response
{
class Base;
}  // namespace response
}  // namespace rpc
}  // namespace opentxs

namespace opentxs
{
auto translate(rpc::AccountEventType type) noexcept -> proto::AccountEventType;
auto translate(rpc::AccountType type) noexcept -> proto::AccountType;
auto translate(rpc::CommandType type) noexcept -> proto::RPCCommandType;
auto translate(rpc::ContactEventType type) noexcept -> proto::ContactEventType;
auto translate(rpc::PaymentType type) noexcept -> proto::RPCPaymentType;
auto translate(rpc::PushType type) noexcept -> proto::RPCPushType;
auto translate(rpc::ResponseCode type) noexcept -> proto::RPCResponseCode;

auto translate(proto::AccountEventType type) noexcept -> rpc::AccountEventType;
auto translate(proto::AccountType type) noexcept -> rpc::AccountType;
auto translate(proto::ContactEventType type) noexcept -> rpc::ContactEventType;
auto translate(proto::RPCCommandType type) noexcept -> rpc::CommandType;
auto translate(proto::RPCPaymentType type) noexcept -> rpc::PaymentType;
auto translate(proto::RPCPushType type) noexcept -> rpc::PushType;
auto translate(proto::RPCResponseCode type) noexcept -> rpc::ResponseCode;
}  // namespace opentxs

namespace opentxs::rpc::internal
{
struct RPC {
    virtual auto Process(const proto::RPCCommand& command) const
        -> proto::RPCResponse = 0;
    virtual auto Process(const request::Base& command) const
        -> std::unique_ptr<response::Base> = 0;

    virtual ~RPC() = default;
};
}  // namespace opentxs::rpc::internal
