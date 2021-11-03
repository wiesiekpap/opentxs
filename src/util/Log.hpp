// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <utility>

#include "internal/util/Log.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
struct Log::Imp final : public internal::Log {
    struct Logger {
        using Source = std::pair<OTZMQPushSocket, std::stringstream>;

        std::atomic<int> verbosity_{-1};
        std::atomic<bool> running_{false};
    };

    static Logger logger_;

    auto operator()(const char* in) const noexcept -> const opentxs::Log&;

    [[noreturn]] auto Assert(
        const char* file,
        const std::size_t line,
        const char* message) const noexcept -> void;
    auto Flush() const noexcept -> void;
    auto Trace(const char* file, const std::size_t line, const char* message)
        const noexcept -> void;

    Imp(const int logLevel, opentxs::Log& parent) noexcept;

    ~Imp() final = default;

private:
    const int level_;
    opentxs::Log& parent_;

    static auto get_buffer(std::string& id) noexcept -> Logger::Source&;

    auto send(const bool terminate) const noexcept -> void;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs
