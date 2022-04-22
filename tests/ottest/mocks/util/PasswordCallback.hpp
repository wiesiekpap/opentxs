// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gmock/gmock.h>
#include <opentxs/opentxs.hpp>

namespace common::mocks::util
{
class PasswordCallbackMock : public opentxs::PasswordCallback
{
    MOCK_METHOD(
        void,
        runOne,
        (const char* szDisplay,
         opentxs::Secret& theOutput,
         const opentxs::UnallocatedCString& key),
        (const, override)){};
    MOCK_METHOD(
        void,
        runTwo,
        (const char* szDisplay,
         opentxs::Secret& theOutput,
         const opentxs::UnallocatedCString& key),
        (const, override)){};
};
}  // namespace common::mocks::util
