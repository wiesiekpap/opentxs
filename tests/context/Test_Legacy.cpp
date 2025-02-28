// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <type_traits>

#include "internal/api/Legacy.hpp"

class Filename : public ::testing::Test
{
};

TEST_F(Filename, GetFilenameBin)
{
    std::string exp{"filename.bin"};
    std::string s{opentxs::api::Legacy::GetFilenameBin("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameBin_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameBin("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameBin("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameBin(nullptr).empty());
}

TEST_F(Filename, GetFilenameA)
{
    std::string exp{"filename.a"};
    std::string s{opentxs::api::Legacy::GetFilenameA("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameA_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameA("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameA("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameA(nullptr).empty());
}

TEST_F(Filename, GetFilenameR)
{
    std::string exp{"filename.r"};
    std::string s{opentxs::api::Legacy::GetFilenameR("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameR_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameR("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameR("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameR(nullptr).empty());
}

TEST_F(Filename, GetFilenameRct)
{
    {
        std::string exp{"123.rct"};
        std::string s{opentxs::api::Legacy::GetFilenameRct(123)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
    {
        std::string exp{"0.rct"};
        std::string s{opentxs::api::Legacy::GetFilenameRct(000)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
}

TEST_F(Filename, getFilenameRct_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameRct(-1).empty());
}

TEST_F(Filename, GetFilenameCrn)
{
    {
        std::string exp{"123.crn"};
        static_assert(
            std::is_same_v<int64_t, opentxs::TransactionNumber>,
            "type is not matching");  // detect if type change
        std::string s{opentxs::api::Legacy::GetFilenameCrn(123)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
    {
        std::string exp{"0.crn"};
        std::string s{opentxs::api::Legacy::GetFilenameCrn(000)};
        ASSERT_STREQ(s.c_str(), exp.c_str());
    }
}

TEST_F(Filename, getFilenameCrn_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameCrn(-1).empty());
}

TEST_F(Filename, GetFilenameSuccess)
{
    std::string exp{"filename.success"};
    std::string s{opentxs::api::Legacy::GetFilenameSuccess("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameSuccess_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameSuccess("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameSuccess("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameSuccess(nullptr).empty());
}

TEST_F(Filename, GetFilenameFail)
{
    std::string exp{"filename.fail"};
    std::string s{opentxs::api::Legacy::GetFilenameFail("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameFail_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameFail("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameFail("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameFail(nullptr).empty());
}

TEST_F(Filename, GetFilenameError)
{
    std::string exp{"filename.error"};
    std::string s{opentxs::api::Legacy::GetFilenameError("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameError_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError(nullptr).empty());
}

TEST_F(Filename, GetFilenameLst)
{
    std::string exp{"filename.lst"};
    std::string s{opentxs::api::Legacy::GetFilenameLst("filename")};
    ASSERT_STREQ(s.c_str(), exp.c_str());
}

TEST_F(Filename, getFilenameLst_invalid_input)
{
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("-1").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError("").empty());
    ASSERT_TRUE(opentxs::api::Legacy::GetFilenameError(nullptr).empty());
}
