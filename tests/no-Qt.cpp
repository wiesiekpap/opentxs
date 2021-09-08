// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Basic.hpp"  // IWYU pragma: associated

#include <thread>

namespace ottest
{
auto GetQT() noexcept -> QObject* { return nullptr; }

auto StartQT(bool) noexcept -> std::thread { return {}; }

auto StopQT() noexcept -> void {}
}  // namespace ottest
