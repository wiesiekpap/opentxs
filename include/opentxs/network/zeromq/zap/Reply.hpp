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
class OPENTXS_EXPORT Reply : virtual public zeromq::Message
{
public:
    static auto Factory(
        const Request& request,
        const zap::Status& code = zap::Status::Unknown,
        const std::string& status = "",
        const std::string& userID = "",
        const Data& metadata = Data::Factory(),
        const std::string& version = "1.0") -> Pimpl<Reply>;

    virtual auto Code() const -> zap::Status = 0;
    virtual auto Debug() const -> std::string = 0;
    virtual auto Metadata() const -> OTData = 0;
    virtual auto RequestID() const -> OTData = 0;
    virtual auto Status() const -> std::string = 0;
    virtual auto UserID() const -> std::string = 0;
    virtual auto Version() const -> std::string = 0;

    virtual auto SetCode(const zap::Status& code) -> bool = 0;
    virtual auto SetMetadata(const Data& metadata) -> bool = 0;
    virtual auto SetStatus(const std::string& status) -> bool = 0;
    virtual auto SetUserID(const std::string& userID) -> bool = 0;

    ~Reply() override = default;

protected:
    Reply() = default;

private:
    friend OTZMQZAPReply;

#ifndef _WIN32
    auto clone() const -> Reply* override = 0;
#endif

    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;
};
}  // namespace zap
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
