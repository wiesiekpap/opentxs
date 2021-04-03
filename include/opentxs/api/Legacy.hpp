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
    static std::string SuggestFolder(const std::string& app) noexcept;

    OPENTXS_EXPORT virtual const char* Account() const noexcept = 0;
    OPENTXS_EXPORT virtual bool AppendFile(
        String& out,
        const String& base,
        const String& file) const noexcept = 0;
    OPENTXS_EXPORT virtual bool AppendFolder(
        String& out,
        const String& base,
        const String& folder) const noexcept = 0;
    OPENTXS_EXPORT virtual bool BuildFolderPath(
        const String& path) const noexcept = 0;
    OPENTXS_EXPORT virtual bool BuildFilePath(
        const String& path) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ClientConfigFilePath(
        const int instance) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ClientDataFolder(
        const int instance) const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Common() const noexcept = 0;
    OPENTXS_EXPORT virtual bool ConfirmCreateFolder(
        const String& path) const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Contract() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Cron() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* ExpiredBox() const noexcept = 0;
    OPENTXS_EXPORT virtual bool FileExists(
        const String& path,
        std::size_t& size) const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Inbox() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Market() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Mint() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Nym() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Nymbox() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string OpentxsConfigFilePath()
        const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Outbox() const noexcept = 0;
    OPENTXS_EXPORT virtual bool PathExists(
        const String& path) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string PIDFilePath() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* PaymentInbox() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* Receipt() const noexcept = 0;
    OPENTXS_EXPORT virtual const char* RecordBox() const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ServerConfigFilePath(
        const int instance) const noexcept = 0;
    OPENTXS_EXPORT virtual std::string ServerDataFolder(
        const int instance) const noexcept = 0;

    virtual ~Legacy() = default;

protected:
    Legacy() noexcept = default;

private:
    Legacy(const Legacy&) = delete;
    Legacy(Legacy&&) = delete;
    Legacy& operator=(const Legacy&) = delete;
    Legacy& operator=(Legacy&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
