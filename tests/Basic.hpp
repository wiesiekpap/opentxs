// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string>
#include <thread>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"

namespace ot = opentxs;

namespace opentxs
{
class Options;
}  // namespace opentxs

class QObject;

namespace ottest
{
auto Args(bool lowlevel = false) noexcept -> const ot::Options&;
auto GetQT() noexcept -> QObject*;
auto Home() noexcept -> const std::string&;
auto StartQT(bool lowlevel = false) noexcept -> std::thread;
auto StopQT() noexcept -> void;
auto WipeHome() noexcept -> void;
}  // namespace ottest
