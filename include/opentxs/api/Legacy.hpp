// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_LEGACY_HPP
#define OPENTXS_API_LEGACY_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

namespace opentxs
{
class String;
}  // namespace opentxs

namespace opentxs
{
namespace api
{
class Legacy
{
public:
    static auto SuggestFolder(const std::string& app) noexcept -> std::string;

    virtual auto Account() const noexcept -> const char* = 0;
    virtual auto AppendFile(String& out, const String& base, const String& file)
        const noexcept -> bool = 0;
    virtual auto AppendFolder(
        String& out,
        const String& base,
        const String& folder) const noexcept -> bool = 0;
    virtual auto BuildFolderPath(const String& path) const noexcept -> bool = 0;
    virtual auto BuildFilePath(const String& path) const noexcept -> bool = 0;
    virtual auto ClientConfigFilePath(const int instance) const noexcept
        -> std::string = 0;
    virtual auto ClientDataFolder(const int instance) const noexcept
        -> std::string = 0;
    virtual auto Common() const noexcept -> const char* = 0;
    virtual auto ConfirmCreateFolder(const String& path) const noexcept
        -> bool = 0;
    virtual auto Contract() const noexcept -> const char* = 0;
    virtual auto Cron() const noexcept -> const char* = 0;
    virtual auto ExpiredBox() const noexcept -> const char* = 0;
    virtual auto FileExists(const String& path, std::size_t& size)
        const noexcept -> bool = 0;
    virtual auto Inbox() const noexcept -> const char* = 0;
    virtual auto Market() const noexcept -> const char* = 0;
    virtual auto Mint() const noexcept -> const char* = 0;
    virtual auto Nym() const noexcept -> const char* = 0;
    virtual auto Nymbox() const noexcept -> const char* = 0;
    virtual auto OpentxsConfigFilePath() const noexcept -> std::string = 0;
    virtual auto Outbox() const noexcept -> const char* = 0;
    virtual auto PathExists(const String& path) const noexcept -> bool = 0;
    virtual auto PIDFilePath() const noexcept -> std::string = 0;
    virtual auto PaymentInbox() const noexcept -> const char* = 0;
    virtual auto Receipt() const noexcept -> const char* = 0;
    virtual auto RecordBox() const noexcept -> const char* = 0;
    virtual auto ServerConfigFilePath(const int instance) const noexcept
        -> std::string = 0;
    virtual auto ServerDataFolder(const int instance) const noexcept
        -> std::string = 0;

    virtual ~Legacy() = default;

protected:
    Legacy() noexcept = default;

private:
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    auto operator=(const Legacy&) -> Legacy& = delete;
    auto operator=(Legacy&&) -> Legacy& = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
