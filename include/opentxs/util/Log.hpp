// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstddef>
#include <string>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
namespace identifier
{
class Nym;
class Server;
class UnitDefinition;
}  // namespace identifier

namespace internal
{
class Log;
}  // namespace internal

class Amount;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Log
{
public:
    struct Imp;

    auto operator()() const noexcept -> const Log&;
    auto operator()(const char* in) const noexcept -> const Log&;
    auto operator()(char* in) const noexcept -> const Log&;
    auto operator()(const std::string& in) const noexcept -> const Log&;
    auto operator()(const std::chrono::nanoseconds& in) const noexcept
        -> const Log&;
    auto operator()(const OTString& in) const noexcept -> const Log&;
    auto operator()(const OTStringXML& in) const noexcept -> const Log&;
    auto operator()(const OTArmored& in) const noexcept -> const Log&;
    auto operator()(const Amount& in) const noexcept -> const Log&;
    auto operator()(const String& in) const noexcept -> const Log&;
    auto operator()(const StringXML& in) const noexcept -> const Log&;
    auto operator()(const Armored& in) const noexcept -> const Log&;
    auto operator()(const OTIdentifier& in) const noexcept -> const Log&;
    auto operator()(const Identifier& in) const noexcept -> const Log&;
    auto operator()(const OTNymID& in) const noexcept -> const Log&;
    auto operator()(const identifier::Nym& in) const noexcept -> const Log&;
    auto operator()(const OTServerID& in) const noexcept -> const Log&;
    auto operator()(const identifier::Server& in) const noexcept -> const Log&;
    auto operator()(const OTUnitID& in) const noexcept -> const Log&;
    auto operator()(const identifier::UnitDefinition& in) const noexcept
        -> const Log&;
    auto operator()(const Time in) const noexcept -> const Log&;
    template <typename T>
    auto operator()(const T& in) const noexcept -> const Log&
    {
        return this->operator()(std::to_string(in));
    }
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> const internal::Log&;

    [[noreturn]] auto Assert(
        const char* file,
        const std::size_t line,
        const char* message) const noexcept -> void;
    [[noreturn]] auto Assert(const char* file, const std::size_t line)
        const noexcept -> void;
    auto Flush() const noexcept -> void;
    OPENTXS_NO_EXPORT auto Internal() noexcept -> internal::Log&;
    auto Trace(const char* file, const std::size_t line, const char* message)
        const noexcept -> void;
    auto Trace(const char* file, const std::size_t line) const noexcept -> void;

    explicit Log(const int logLevel) noexcept;

    ~Log();

private:
    Imp* imp_;

    Log() = delete;
    Log(const Log&) = delete;
    Log(Log&&) = delete;
    auto operator=(const Log&) -> Log& = delete;
    auto operator=(Log&&) -> Log& = delete;
};

OPENTXS_EXPORT auto LogConsole() noexcept -> Log&;
OPENTXS_EXPORT auto LogDebug() noexcept -> Log&;
OPENTXS_EXPORT auto LogDetail() noexcept -> Log&;
OPENTXS_EXPORT auto LogError() noexcept -> Log&;
OPENTXS_EXPORT auto LogInsane() noexcept -> Log&;
OPENTXS_EXPORT auto LogTrace() noexcept -> Log&;
OPENTXS_EXPORT auto LogVerbose() noexcept -> Log&;
OPENTXS_EXPORT auto PrintStackTrace() noexcept -> std::string;
}  // namespace opentxs
