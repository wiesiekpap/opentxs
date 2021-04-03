// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_FLAG_HPP
#define OPENTXS_CORE_FLAG_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/Pimpl.hpp"

namespace opentxs
{
class Flag;

using OTFlag = Pimpl<Flag>;
}  // namespace opentxs

namespace opentxs
{
/** Wrapper for a std::atomic<bool> */
class Flag
{
public:
    static OTFlag Factory(const bool state);

    virtual operator bool() const = 0;

    /** Returns true if new state differs from previous state */
    virtual bool Off() = 0;
    /** Returns true if new state differs from previous state */
    virtual bool On() = 0;
    /** Returns previous state */
    virtual bool Set(const bool value) = 0;
    /** Returns previous state */
    virtual bool Toggle() = 0;

    virtual ~Flag() = default;

protected:
    Flag() = default;

private:
    friend OTFlag;

    virtual Flag* clone() const = 0;

    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    Flag& operator=(const Flag&) = delete;
    Flag& operator=(Flag&&) = delete;
};
}  // namespace opentxs
#endif
