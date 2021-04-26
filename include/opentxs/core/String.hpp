// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_STRING_HPP
#define OPENTXS_CORE_STRING_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdarg>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "opentxs/Bytes.hpp"
#include "opentxs/Pimpl.hpp"

#ifdef __GNUC__
#define ATTR_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#define ATTR_PRINTF(a, b)
#endif

#ifdef _MSC_VER
#define PRI_SIZE "Iu"
#else
#define PRI_SIZE "zu"
#endif

namespace opentxs
{
class Armored;
class Contract;
class Identifier;
class NymFile;
class Signature;
class String;

using OTString = Pimpl<String>;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT String
{
public:
    using List = std::list<std::string>;
    using Map = std::map<std::string, std::string>;

    static opentxs::Pimpl<opentxs::String> Factory();
    static opentxs::Pimpl<opentxs::String> Factory(const Armored& value);
    static opentxs::Pimpl<opentxs::String> Factory(const Signature& value);
    static opentxs::Pimpl<opentxs::String> Factory(const Contract& value);
    static opentxs::Pimpl<opentxs::String> Factory(const Identifier& value);
    static opentxs::Pimpl<opentxs::String> Factory(const NymFile& value);
    static opentxs::Pimpl<opentxs::String> Factory(const char* value);
    static opentxs::Pimpl<opentxs::String> Factory(const std::string& value);
    static opentxs::Pimpl<opentxs::String> Factory(
        const char* value,
        std::size_t size);

    static std::string LongToString(const std::int64_t& lNumber);
    static std::string replace_chars(
        const std::string& str,
        const std::string& charsFrom,
        const char& charTo);
    static std::size_t safe_strlen(const char* s, std::size_t max);
    static std::int32_t StringToInt(const std::string& number);
    static std::int64_t StringToLong(const std::string& number);
    static std::uint32_t StringToUint(const std::string& number);
    static std::uint64_t StringToUlong(const std::string& number);
    static std::string& trim(std::string& str);
    static std::string UlongToString(const std::uint64_t& uNumber);
    static bool vformat(const char* fmt, std::va_list* pvl, std::string& s)
        ATTR_PRINTF(1, 0);

    virtual bool operator>(const String& rhs) const = 0;
    virtual bool operator<(const String& rhs) const = 0;
    virtual bool operator<=(const String& rhs) const = 0;
    virtual bool operator>=(const String& rhs) const = 0;
    virtual bool operator==(const String& rhs) const = 0;

    virtual bool At(std::uint32_t index, char& c) const = 0;
    virtual ReadView Bytes() const noexcept = 0;
    virtual bool Compare(const char* compare) const = 0;
    virtual bool Compare(const String& compare) const = 0;
    virtual bool Contains(const char* compare) const = 0;
    virtual bool Contains(const String& compare) const = 0;
    virtual bool empty() const = 0;
    virtual bool Exists() const = 0;
    virtual const char* Get() const = 0;
    virtual std::uint32_t GetLength() const = 0;
    virtual std::int32_t ToInt() const = 0;
    virtual bool TokenizeIntoKeyValuePairs(Map& map) const = 0;
    virtual std::int64_t ToLong() const = 0;
    virtual std::uint32_t ToUint() const = 0;
    virtual std::uint64_t ToUlong() const = 0;
    virtual void WriteToFile(std::ostream& ofs) const = 0;

    virtual void Concatenate(const char* arg, ...) ATTR_PRINTF(2, 3) = 0;
    virtual void Concatenate(const String& data) = 0;
    virtual void ConvertToUpperCase() = 0;
    virtual bool DecodeIfArmored(bool escapedIsAllowed = true) = 0;
    virtual void Format(const char* fmt, ...) ATTR_PRINTF(2, 3) = 0;
    /** For a straight-across, exact-size copy of bytes. Source not expected to
     * be null-terminated. */
    virtual bool MemSet(const char* mem, std::uint32_t size) = 0;
    virtual void Release() = 0;
    /** new_string MUST be at least nEnforcedMaxLength in size if
    nEnforcedMaxLength is passed in at all.
    That's because this function forces the null terminator at that length,
    minus 1. For example, if the max is set to 10, then the valid range is 0..9.
    Therefore 9 (10 minus 1) is where the nullptr terminator goes. */
    virtual void Set(const char* data, std::uint32_t enforcedMaxLength = 0) = 0;
    virtual void Set(const String& data) = 0;
    /** true  == there are more lines to read.
    false == this is the last line. Like EOF. */
    virtual bool sgets(char* buffer, std::uint32_t size) = 0;
    virtual char sgetc() = 0;
    virtual void swap(String& rhs) = 0;
    virtual void reset() = 0;
    virtual AllocateOutput WriteInto() noexcept = 0;

    virtual ~String() = default;

protected:
    String() = default;

private:
    friend OTString;
    friend std::ostream& operator<<(std::ostream& os, const String& obj);

#ifdef _WIN32
public:
#endif
    virtual String* clone() const = 0;
#ifdef _WIN32
private:
#endif

    String(String&& rhs) = delete;
    String& operator=(const String& rhs) = delete;
    String& operator=(String&& rhs) = delete;
};
}  // namespace opentxs
#endif
