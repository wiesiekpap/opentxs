// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_API_SETTINGS_HPP
#define OPENTXS_API_SETTINGS_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string>

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
namespace api
{
class Settings
{
public:
    OPENTXS_EXPORT virtual void SetConfigFilePath(
        const String& strConfigFilePath) const = 0;
    OPENTXS_EXPORT virtual bool HasConfigFilePath() const = 0;

    // Core (Public Load and Save)
    OPENTXS_EXPORT virtual bool Load() const = 0;
    OPENTXS_EXPORT virtual bool Save() const = 0;

    OPENTXS_EXPORT virtual const Flag& IsLoaded() const = 0;

    // Configuration Helpers
    //

    // Core (Reset Config, and Check if Config is empty)
    OPENTXS_EXPORT virtual bool IsEmpty() const = 0;

    // Check Only (get value of key from configuration, if the key exists, then
    // out_bKeyExist will be true.)
    OPENTXS_EXPORT virtual bool Check_str(
        const String& strSection,
        const String& strKey,
        String& out_strResult,
        bool& out_bKeyExist) const = 0;
    OPENTXS_EXPORT virtual bool Check_long(
        const String& strSection,
        const String& strKey,
        std::int64_t& out_lResult,
        bool& out_bKeyExist) const = 0;
    OPENTXS_EXPORT virtual bool Check_bool(
        const String& strSection,
        const String& strKey,
        bool& out_bResult,
        bool& out_bKeyExist) const = 0;

    // Set Only (set new or update value, out_bNewOrUpdate will be true if the
    // value changes.)
    OPENTXS_EXPORT virtual bool Set_str(
        const String& strSection,
        const String& strKey,
        const String& strValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const = 0;
    OPENTXS_EXPORT virtual bool Set_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const = 0;
    OPENTXS_EXPORT virtual bool Set_bool(
        const String& strSection,
        const String& strKey,
        const bool& bValue,
        bool& out_bNewOrUpdate,
        const String& strComment = String::Factory()) const = 0;

    // Check for a Section, if the section dosn't exist, it will be made and
    // out_bIsNewSection will be true.)
    OPENTXS_EXPORT virtual bool CheckSetSection(
        const String& strSection,
        const String& strComment,
        bool& out_bIsNewSection) const = 0;

    // Check for Key, and returns if the key exists, otherwise will set the
    // default key. If the default key is set, then out_bIsNew will be true.)
    OPENTXS_EXPORT virtual bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        std::string& out_strResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const = 0;
    OPENTXS_EXPORT virtual bool CheckSet_str(
        const String& strSection,
        const String& strKey,
        const String& strDefault,
        String& out_strResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const = 0;
    OPENTXS_EXPORT virtual bool CheckSet_long(
        const String& strSection,
        const String& strKey,
        const std::int64_t& lDefault,
        std::int64_t& out_lResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const = 0;
    OPENTXS_EXPORT virtual bool CheckSet_bool(
        const String& strSection,
        const String& strKey,
        const bool& bDefault,
        bool& out_bResult,
        bool& out_bIsNew,
        const String& strComment = String::Factory()) const = 0;

    // Set Option helper function for setting bool's
    OPENTXS_EXPORT virtual bool SetOption_bool(
        const String& strSection,
        const String& strKey,
        bool& bVariableName) const = 0;

    OPENTXS_EXPORT virtual bool Reset() = 0;

    OPENTXS_EXPORT virtual ~Settings() = default;

protected:
    Settings() = default;

private:
    Settings(const Settings&) = delete;
    Settings(Settings&&) = delete;
    Settings& operator=(const Settings&) = delete;
    Settings& operator=(Settings&&) = delete;
};
}  // namespace api
}  // namespace opentxs
#endif
