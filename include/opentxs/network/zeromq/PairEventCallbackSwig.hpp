// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_HPP
#define OPENTXS_NETWORK_ZEROMQ_PAIREVENTCALLBACKSWIG_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

namespace opentxs
{
class PairEventCallbackSwig
{
public:
    virtual void ProcessRename(const std::string& issuer) const = 0;
    virtual void ProcessStoreSecret(const std::string& issuer) const = 0;

    virtual ~PairEventCallbackSwig() = default;

protected:
    PairEventCallbackSwig() = default;

private:
    PairEventCallbackSwig(const PairEventCallbackSwig&) = delete;
    PairEventCallbackSwig(PairEventCallbackSwig&&) = default;
    auto operator=(const PairEventCallbackSwig&)
        -> PairEventCallbackSwig& = delete;
    auto operator=(PairEventCallbackSwig&&) -> PairEventCallbackSwig& = default;
};
}  // namespace opentxs
#endif
