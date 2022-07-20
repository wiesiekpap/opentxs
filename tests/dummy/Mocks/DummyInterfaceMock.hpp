// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <gmock/gmock.h>

#include "dummy/Interfaces/DummyInterface.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
namespace ottest::mock
{
class Interface : public ::ottest::Interface
{
public:
    Interface()
        : ::ottest::Interface()
    {
    }
    // NOLINTBEGIN(modernize-use-trailing-return-type)
    MOCK_METHOD(bool, someFunction, (const int& input), (override));
    MOCK_METHOD(bool, someConstFunction, (const int& input), (override));
    MOCK_METHOD(
        bool,
        someFunctionWithNoExcept,
        (const int& input),
        (noexcept, override));
    MOCK_METHOD(
        bool,
        someConstFunctionWithNoExcept,
        (const int& input),
        (const, noexcept, override));
    // NOLINTEND(modernize-use-trailing-return-type)
};
}  // namespace ottest::mock
#pragma GCC diagnostic pop
