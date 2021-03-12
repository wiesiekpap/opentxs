// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACKSWIG_HPP
#define OPENTXS_NETWORK_ZEROMQ_LISTENCALLBACKSWIG_HPP

#include "opentxs/Forward.hpp"  // IWYU pragma: associated

#ifdef SWIG
// clang-format off
%feature("director") ListenCallbackSwig;
// clang-format on
#endif  // SWIG

namespace opentxs
{
class ListenCallbackSwig
{
public:
    virtual void Process(network::zeromq::Message& message) const = 0;

    virtual ~ListenCallbackSwig() = default;

protected:
    ListenCallbackSwig() = default;

private:
    ListenCallbackSwig(const ListenCallbackSwig&) = delete;
    ListenCallbackSwig(ListenCallbackSwig&&) = default;
    ListenCallbackSwig& operator=(const ListenCallbackSwig&) = delete;
    ListenCallbackSwig& operator=(ListenCallbackSwig&&) = default;
};
}  // namespace opentxs
#endif
