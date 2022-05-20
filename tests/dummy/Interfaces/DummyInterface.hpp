// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

namespace ottest
{
class Interface
{
public:
    virtual auto someFunction(const int& input) -> bool = 0;
    virtual auto someConstFunction(const int& input) -> bool = 0;
    virtual auto someFunctionWithNoExcept(const int& input) noexcept
        -> bool = 0;
    virtual auto someConstFunctionWithNoExcept(const int& input) const noexcept
        -> bool = 0;

    Interface(const Interface&) = delete;
    Interface(Interface&&) = delete;
    auto operator=(const Interface&) -> Interface& = delete;
    auto operator=(const Interface&&) -> Interface& = delete;

    virtual ~Interface() = default;

protected:
    Interface() = default;
};
}  // namespace ottest
