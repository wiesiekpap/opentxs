// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: begin_exports
#include "opentxs/Version.hpp"  // IWYU pragma: associated
// IWYU pragma: end_exports

#define PAYMENT_CODE_VERSION 1
#define PEER_MESSAGE_VERSION 2
#define PEER_PAYMENT_VERSION 5
#define PEER_CASH_VERSION 7
#define PEER_OBJECT_PEER_REQUEST 7
#define PEER_OBJECT_PEER_REPLY 7
#define OT_CONTACT_VERSION 3
#define CONTACT_CONTACT_DATA_VERSION 6
#define MESSAGE_SEND_ERROR -1
#define MESSAGE_NOT_SENT_NO_ERROR 0
#define MESSAGE_SENT 1
#define REPLY_NOT_RECEIVED -1
#define MESSAGE_SUCCESS_FALSE 0
#define MESSAGE_SUCCESS_TRUE 1
#define FIRST_REQUEST_NUMBER 1

namespace opentxs
{
inline namespace v1
{
namespace api
{
namespace crypto
{
namespace blank
{
}  // namespace blank

namespace blockchain
{
}  // namespace blockchain

namespace imp
{
}  // namespace imp
}  // namespace crypto

namespace imp
{
}  // namespace imp

namespace network
{
namespace asio
{
}  // namespace asio

namespace blockchain
{
namespace database
{
namespace common
{
}  // namespace common
}  // namespace database
}  // namespace blockchain

namespace imp
{
}  // namespace imp
}  // namespace network

namespace session
{
namespace activity
{
}  // namespace activity

namespace base
{
}  // namespace base

namespace client
{
}  // namespace client

namespace imp
{
}  // namespace imp

namespace server
{
}  // namespace server

namespace ui
{
}  // namespace ui
}  // namespace session
}  // namespace api

namespace factory
{
}  // namespace factory
}  // namespace v1
}  // namespace opentxs
