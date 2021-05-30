// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_LOGSOURCE_HPP
#define OPENTXS_CORE_LOGSOURCE_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"

namespace opentxs
{
class OPENTXS_EXPORT LogSource
{
public:
    static void SetVerbosity(const int level) noexcept;
    static void Shutdown() noexcept;
    static const LogSource& StartLog(
        const LogSource& source,
        const std::string& function) noexcept;

    const LogSource& operator()() const noexcept;
    const LogSource& operator()(const char* in) const noexcept;
    const LogSource& operator()(char* in) const noexcept;
    const LogSource& operator()(const std::string& in) const noexcept;
    const LogSource& operator()(const OTString& in) const noexcept;
    const LogSource& operator()(const OTStringXML& in) const noexcept;
    const LogSource& operator()(const OTArmored& in) const noexcept;
    const LogSource& operator()(const String& in) const noexcept;
    const LogSource& operator()(const StringXML& in) const noexcept;
    const LogSource& operator()(const Armored& in) const noexcept;
    const LogSource& operator()(const OTIdentifier& in) const noexcept;
    const LogSource& operator()(const Identifier& in) const noexcept;
    const LogSource& operator()(const OTNymID& in) const noexcept;
    const LogSource& operator()(const identifier::Nym& in) const noexcept;
    const LogSource& operator()(const OTServerID& in) const noexcept;
    const LogSource& operator()(const identifier::Server& in) const noexcept;
    const LogSource& operator()(const OTUnitID& in) const noexcept;
    const LogSource& operator()(
        const identifier::UnitDefinition& in) const noexcept;
    const LogSource& operator()(const Time in) const noexcept;
    template <typename T>
    const LogSource& operator()(const T& in) const noexcept
    {
        return this->operator()(std::to_string(in));
    }

    [[noreturn]] void Assert(
        const char* file,
        const std::size_t line,
        const char* message) const noexcept;
    void Flush() const noexcept;
    void Trace(const char* file, const std::size_t line, const char* message)
        const noexcept;

    explicit LogSource(const int logLevel) noexcept;

    ~LogSource() = default;

private:
    using Source = std::pair<OTZMQPushSocket, std::stringstream>;

    static std::atomic<int> verbosity_;
    static std::atomic<bool> running_;
    static std::mutex buffer_lock_;
    static std::map<std::thread::id, Source> buffer_;

    const int level_{-1};

    static Source& get_buffer(std::string& id) noexcept;

    void send(const bool terminate) const noexcept;

    LogSource() = delete;
    LogSource(const LogSource&) = delete;
    LogSource(LogSource&&) = delete;
    LogSource& operator=(const LogSource&) = delete;
    LogSource& operator=(LogSource&&) = delete;
};
}  // namespace opentxs
#endif
