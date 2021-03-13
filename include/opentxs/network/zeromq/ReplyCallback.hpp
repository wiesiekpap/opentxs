// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP
#define OPENTXS_NETWORK_ZEROMQ_REPLYCALLBACK_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#include <functional>

#include "opentxs/Pimpl.hpp"

#ifdef SWIG
// clang-format off
%ignore opentxs::network::zeromq::ReplyCallback::Factory;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>::Pimpl(opentxs::network::zeromq::ReplyCallback const &);
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>::operator opentxs::network::zeromq::ReplyCallback&;
%ignore opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>::operator const opentxs::network::zeromq::ReplyCallback &;
%rename(assign) operator=(const opentxs::network::zeromq::ReplyCallback&);
%rename(ZMQReplyCallback) opentxs::network::zeromq::ReplyCallback;
%template(OTZMQReplyCallback) opentxs::Pimpl<opentxs::network::zeromq::ReplyCallback>;
// clang-format on
#endif  // SWIG

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

using OTZMQReplyCallback = Pimpl<network::zeromq::ReplyCallback>;
}  // namespace opentxs

namespace opentxs
{
namespace network
{
namespace zeromq
{
class ReplyCallback
{
public:
    using ReceiveCallback = std::function<OTZMQMessage(const Message&)>;

    OPENTXS_EXPORT static OTZMQReplyCallback Factory(ReceiveCallback callback);

    OPENTXS_EXPORT virtual Pimpl<opentxs::network::zeromq::Message> Process(
        const Message& message) const = 0;

    OPENTXS_EXPORT virtual ~ReplyCallback() = default;

protected:
    ReplyCallback() = default;

private:
    friend OTZMQReplyCallback;

    virtual ReplyCallback* clone() const = 0;

    ReplyCallback(const ReplyCallback&) = delete;
    ReplyCallback(ReplyCallback&&) = default;
    ReplyCallback& operator=(const ReplyCallback&) = delete;
    ReplyCallback& operator=(ReplyCallback&&) = default;
};
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
#endif
