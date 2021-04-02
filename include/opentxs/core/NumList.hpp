// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NUMLIST_HPP
#define OPENTXS_CORE_NUMLIST_HPP

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <set>
#include <string>

namespace opentxs
{
class String;

/** Useful for storing a std::set of longs, serializing to/from comma-separated
 * string, And easily being able to add/remove/verify the individual transaction
 * numbers that are there. (Used by OTTransaction::blank and
 * OTTransaction::successNotice.) Also used in OTMessage, for storing lists of
 * acknowledged request numbers. */
class NumList
{
    std::set<std::int64_t> m_setData;

    /** private for security reasons, used internally only by a function that
     * knows the string length already. if false, means the numbers were already
     * there. (At least one of them.) */
    bool Add(const char* szfNumbers);

public:
    explicit OPENTXS_EXPORT NumList(const std::set<std::int64_t>& theNumbers);
    explicit OPENTXS_EXPORT NumList(std::set<std::int64_t>&& theNumbers);
    explicit OPENTXS_EXPORT NumList(const String& strNumbers);
    explicit OPENTXS_EXPORT NumList(const std::string& strNumbers);
    explicit OPENTXS_EXPORT NumList(std::int64_t lInput);
    OPENTXS_EXPORT NumList();
    OPENTXS_EXPORT NumList(const NumList&) = default;
    OPENTXS_EXPORT NumList(NumList&&) = default;
    OPENTXS_EXPORT NumList& operator=(const NumList&) = default;
    OPENTXS_EXPORT NumList& operator=(NumList&&) = default;

    OPENTXS_EXPORT ~NumList() = default;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    OPENTXS_EXPORT bool Add(const String& strNumbers);

    /** if false, means the numbers were already there. (At least one of them.)
     */
    OPENTXS_EXPORT bool Add(const std::string& strNumbers);

    /** if false, means the value was already there. */
    OPENTXS_EXPORT bool Add(const std::int64_t& theValue);

    /** if false, means the value was NOT already there. */
    OPENTXS_EXPORT bool Remove(const std::int64_t& theValue);

    /** returns true/false (whether value is already there.) */
    OPENTXS_EXPORT bool Verify(const std::int64_t& theValue) const;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    OPENTXS_EXPORT bool Add(const NumList& theNumList);

    /** if false, means the numbers were already there. (At least one of them.)
     */
    OPENTXS_EXPORT bool Add(const std::set<std::int64_t>& theNumbers);

    /** if false, means the numbers were NOT already there. (At least one of
     * them.) */
    OPENTXS_EXPORT bool Remove(const std::set<std::int64_t>& theNumbers);

    /** True/False, based on whether values are already there. (ALL theNumbers
     * must be present.) */
    OPENTXS_EXPORT bool Verify(const std::set<std::int64_t>& theNumbers) const;

    /** True/False, based on whether OTNumLists MATCH in COUNT and CONTENT (NOT
     * ORDER.) */
    OPENTXS_EXPORT bool Verify(const NumList& rhs) const;

    /** True/False, based on whether ANY of rhs are found in *this. */
    OPENTXS_EXPORT bool VerifyAny(const NumList& rhs) const;

    /** Verify whether ANY of the numbers on *this are found in setData. */
    OPENTXS_EXPORT bool VerifyAny(const std::set<std::int64_t>& setData) const;
    OPENTXS_EXPORT std::int32_t Count() const;
    OPENTXS_EXPORT bool Peek(std::int64_t& lPeek) const;
    OPENTXS_EXPORT bool Pop();

    /** Outputs the numlist as set of numbers. (To iterate OTNumList, call this,
     * then iterate the output.) returns false if the numlist was empty.*/
    OPENTXS_EXPORT bool Output(std::set<std::int64_t>& theOutput) const;

    /** Outputs the numlist as a comma-separated string (for serialization,
     * usually.) returns false if the numlist was empty. */
    OPENTXS_EXPORT bool Output(String& strOutput) const;
    OPENTXS_EXPORT void Release();
};
}  // namespace opentxs
#endif
