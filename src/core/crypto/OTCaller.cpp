// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "opentxs/core/crypto/OTCaller.hpp"  // IWYU pragma: associated

#include "opentxs/core/Log.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/crypto/OTCallback.hpp"

// #define OT_METHOD "opentxs::OTCaller"

namespace opentxs
{
OTCaller::OTCaller()
    : callback_(nullptr)
{
}

void OTCaller::AskOnce(
    const PasswordPrompt& prompt,
    Secret& output,
    const std::string& key)
{
    OT_ASSERT(callback_);

    callback_->runOne(prompt.GetDisplayString(), output, key);
}

void OTCaller::AskTwice(
    const PasswordPrompt& prompt,
    Secret& output,
    const std::string& key)
{
    OT_ASSERT(callback_);

    callback_->runTwo(prompt.GetDisplayString(), output, key);
}

auto OTCaller::HaveCallback() const -> bool { return nullptr != callback_; }

void OTCaller::SetCallback(OTCallback* callback) { callback_ = callback; }

OTCaller::~OTCaller() { callback_ = nullptr; }
}  // namespace opentxs
