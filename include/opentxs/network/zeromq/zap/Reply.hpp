// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_ZAP_REPLY_HPP
#define OPENTXS_NETWORK_ZEROMQ_ZAP_REPLY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/zap/ZAP.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Reply;
class Request;
}  // namespace zap
}  // namespace zeromq
}  // namespace network

using OTZMQZAPReply = Pimpl<network::zeromq::zap::Reply>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace zap
{
class Reply : virtual public zeromq::Message
{
public:
    OPENTXS_EXPORT static Pimpl<Reply> Factory(
        const Request& request,
        const zap::Status& code = zap::Status::Unknown,
        const std::string& status = "",
        const std::string& userID = "",
        const Data& metadata = Data::Factory(),
        const std::string& version = "1.0");

    OPENTXS_EXPORT virtual zap::Status Code() const = 0;
    OPENTXS_EXPORT virtual std::string Debug() const = 0;
    OPENTXS_EXPORT virtual OTData Metadata() const = 0;
    OPENTXS_EXPORT virtual OTData RequestID() const = 0;
    OPENTXS_EXPORT virtual std::string Status() const = 0;
    OPENTXS_EXPORT virtual std::string UserID() const = 0;
    OPENTXS_EXPORT virtual std::string Version() const = 0;

    OPENTXS_EXPORT virtual bool SetCode(const zap::Status& code) = 0;
    OPENTXS_EXPORT virtual bool SetMetadata(const Data& metadata) = 0;
    OPENTXS_EXPORT virtual bool SetStatus(const std::string& status) = 0;
    OPENTXS_EXPORT virtual bool SetUserID(const std::string& userID) = 0;

    OPENTXS_EXPORT ~Reply() override = default;

protected:
    Reply() = default;

private:
    friend OTZMQZAPReply;

#ifndef _WIN32
    Reply* clone() const override = 0;
#endif

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    Reply& operator=(const Reply&) = delete;
    Reply& operator=(Reply&&) = delete;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
