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

class PairEventCallbackSwig;
}  // namespace opentxs

namespace opentxs::network::zeromq::implementation
{
class PairEventCallbackSwig final : virtual public zeromq::PairEventCallback
{
public:
    void Process(zeromq::Message& message) const final;

    ~PairEventCallbackSwig() final;

private:
    friend zeromq::PairEventCallback;

    opentxs::PairEventCallbackSwig* callback_;

    auto clone() const -> PairEventCallbackSwig* final;

    PairEventCallbackSwig(opentxs::PairEventCallbackSwig* callback);
    PairEventCallbackSwig() = delete;
    PairEventCallbackSwig(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig(PairEventCallbackSwig&&) = delete;
    auto operator=(const PairEventCallbackSwig&)
        -> PairEventCallbackSwig& = delete;
    auto operator=(PairEventCallbackSwig&&) -> PairEventCallbackSwig& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
