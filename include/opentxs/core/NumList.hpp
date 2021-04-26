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
class OPENTXS_EXPORT NumList
{
    std::set<std::int64_t> m_setData;

    /** private for security reasons, used internally only by a function that
     * knows the string length already. if false, means the numbers were already
     * there. (At least one of them.) */
    bool Add(const char* szfNumbers);

public:
    explicit NumList(const std::set<std::int64_t>& theNumbers);
    explicit NumList(std::set<std::int64_t>&& theNumbers);
    explicit NumList(const String& strNumbers);
    explicit NumList(const std::string& strNumbers);
    explicit NumList(std::int64_t lInput);
    NumList();
    NumList(const NumList&) = default;
    NumList(NumList&&) = default;
    NumList& operator=(const NumList&) = default;
    NumList& operator=(NumList&&) = default;

    ~NumList() = default;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    bool Add(const String& strNumbers);

    /** if false, means the numbers were already there. (At least one of them.)
     */
    bool Add(const std::string& strNumbers);

    /** if false, means the value was already there. */
    bool Add(const std::int64_t& theValue);

    /** if false, means the value was NOT already there. */
    bool Remove(const std::int64_t& theValue);

    /** returns true/false (whether value is already there.) */
    bool Verify(const std::int64_t& theValue) const;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    bool Add(const NumList& theNumList);

    /** if false, means the numbers were already there. (At least one of them.)
     */
    bool Add(const std::set<std::int64_t>& theNumbers);

    /** if false, means the numbers were NOT already there. (At least one of
     * them.) */
    bool Remove(const std::set<std::int64_t>& theNumbers);

    /** True/False, based on whether values are already there. (ALL theNumbers
     * must be present.) */
    bool Verify(const std::set<std::int64_t>& theNumbers) const;

    /** True/False, based on whether OTNumLists MATCH in COUNT and CONTENT (NOT
     * ORDER.) */
    bool Verify(const NumList& rhs) const;

    /** True/False, based on whether ANY of rhs are found in *this. */
    bool VerifyAny(const NumList& rhs) const;

    /** Verify whether ANY of the numbers on *this are found in setData. */
    bool VerifyAny(const std::set<std::int64_t>& setData) const;
    std::int32_t Count() const;
    bool Peek(std::int64_t& lPeek) const;
    bool Pop();

    /** Outputs the numlist as set of numbers. (To iterate OTNumList, call this,
     * then iterate the output.) returns false if the numlist was empty.*/
    bool Output(std::set<std::int64_t>& theOutput) const;

    /** Outputs the numlist as a comma-separated string (for serialization,
     * usually.) returns false if the numlist was empty. */
    bool Output(String& strOutput) const;
    void Release();
};
}  // namespace opentxs
#endif
