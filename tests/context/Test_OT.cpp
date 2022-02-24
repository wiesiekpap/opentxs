// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <functional>
#include <memory>
#include <stdexcept>

#include "Basic.hpp"
#include "common/mocks/util/PasswordCallbackMock.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordCallback.hpp"
#include "opentxs/util/PasswordCaller.hpp"
#include "util/license/License.hpp"

namespace ottest
{
namespace ot_mocks = common::mocks::util;
using ::testing::StrictMock;

TEST(OT_suite, ShouldThrowAnExceptionDuringGettingUninitializeContext)
{
    const std::string expected = "Context is not initialized";
    std::string error_message;

    try {
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeAndGetValidContext)
{
    const std::string expected = "";
    std::string error_message;

    try {
        opentxs::InitContext();
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeContextWithArgs)
{
    const std::string expected = "";
    std::string error_message;

    try {
        opentxs::InitContext(ottest::Args(true));
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeContextWithInvalidPasswordCallback)
{
    const std::string expected = "";
    std::string error_message;
    try {
        opentxs::InitContext(nullptr);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }
    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldSuccessfullyInitializeContextWithValidPasswordCaller)
{
    const std::string expected = "";
    std::string error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(&caller);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldInitializeContextWithArgsAndInvalidPasswordCallerWithoutThrowingAnException)
{
    const std::string expected = "";
    std::string error_message;

    try {
        opentxs::InitContext(ottest::Args(true), nullptr);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldInitializeContextWithArgsAndValidPasswordCallerWithoutThrowingAnException)
{
    const std::string expected = "";
    std::string error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(ottest::Args(true), &caller);
        [[maybe_unused]] const auto& otx = opentxs::Context();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldDoubleDefaultInitializeContextAndThrowAnException)
{
    const std::string expected = "Context is already initialized";
    std::string error_message;
    try {
        opentxs::InitContext();
        opentxs::InitContext();
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldDoubleInitializeContextWithArgsAndThrowAnException)
{
    const std::string expected = "Context is already initialized";
    std::string error_message;
    try {
        opentxs::InitContext(ottest::Args(true));
        opentxs::InitContext(ottest::Args(true));
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldDoubleInitializeContextWithValidPasswordCallerAndThrowAnException)
{
    const std::string expected = "Context is already initialized";
    std::string error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(&caller);
        opentxs::InitContext(&caller);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldDoubleInitializeContextWithArgsAndValidPasswordCallerAndThrowAnException)
{
    const std::string expected = "Context is already initialized";
    std::string error_message;

    StrictMock<ot_mocks::PasswordCallbackMock> mock;
    opentxs::PasswordCaller caller;
    caller.SetCallback(&mock);

    try {
        opentxs::InitContext(ottest::Args(true), &caller);
        opentxs::InitContext(ottest::Args(true), &caller);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(
    OT_suite,
    ShouldDoubleInitializeContextWithArgsAndNotValidPasswordCallerAndThrowAnException)
{
    const std::string expected = "Context is already initialized";
    std::string error_message;

    try {
        opentxs::InitContext(ottest::Args(true), nullptr);
        opentxs::InitContext(ottest::Args(true), nullptr);
    } catch (const std::runtime_error& er) {
        error_message = er.what();
    }

    EXPECT_EQ(expected, error_message);
    opentxs::Cleanup();
}

TEST(OT_suite, ShouldReturnValidLicenseMap)
{
    const auto expected_license_map = std::invoke([]() {
        auto out = opentxs::LicenseMap{};
        opentxs::license_argon(out);
        opentxs::license_base58(out);
        opentxs::license_base64(out);
        opentxs::license_bech32(out);
        opentxs::license_chaiscript(out);
        opentxs::license_irrxml(out);
        opentxs::license_libguarded(out);
        opentxs::license_lucre(out);
        opentxs::license_opentxs(out);
        opentxs::license_packetcrypt(out);
        opentxs::license_protobuf(out);
        opentxs::license_secp256k1(out);
        opentxs::license_simpleini(out);
        return out;
    });

    EXPECT_EQ(expected_license_map, opentxs::LicenseData());
}

}  // namespace ottest
