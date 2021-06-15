// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>

#include "opentxs/core/crypto/OTCallback.hpp"

namespace opentxs
{
class Factory;
class OTPassword;
class Secret;
}  // namespace opentxs

namespace opentxs::implementation
{
class NullCallback final : virtual public OTCallback
{
public:
    void runOne(const char* display, Secret& output, const std::string& key)
        const final;
    void runTwo(const char* display, Secret& output, const std::string& key)
        const final;

    NullCallback() = default;

    ~NullCallback() final = default;

private:
    friend opentxs::Factory;

    static const std::string password_;

    NullCallback(const NullCallback&) = delete;
    NullCallback(NullCallback&&) = delete;
    auto operator=(const NullCallback&) -> NullCallback& = delete;
    auto operator=(NullCallback&&) -> NullCallback& = delete;
};
}  // namespace opentxs::implementation
