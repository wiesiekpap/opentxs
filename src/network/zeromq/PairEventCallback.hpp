// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/network/zeromq/PairEventCallback.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::implementation
{
class PairEventCallback final : virtual public zeromq::PairEventCallback
{
public:
    void Process(zeromq::Message& message) const final;

    ~PairEventCallback() final;

private:
    friend zeromq::PairEventCallback;

    const zeromq::PairEventCallback::ReceiveCallback callback_;

    auto clone() const -> PairEventCallback* final;

    PairEventCallback(zeromq::PairEventCallback::ReceiveCallback callback);
    PairEventCallback() = delete;
    PairEventCallback(const PairEventCallback&) = delete;
    PairEventCallback(PairEventCallback&&) = delete;
    auto operator=(const PairEventCallback&) -> PairEventCallback& = delete;
    auto operator=(PairEventCallback&&) -> PairEventCallback& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
